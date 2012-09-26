/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2009-2012 Alexander Rieder <alexanderrieder@gmail.com>
 */

#include "maximasession.h"
#include "maximaexpression.h"
#include "maximacompletionobject.h"
#include "maximasyntaxhelpobject.h"
#include "maximahighlighter.h"
#include "maximavariablemodel.h"

#include <QTimer>
#include <QTcpSocket>
#include <QTcpServer>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <signal.h>
#include "settings.h"

#ifdef Q_OS_WIN
  #include <kprocess.h>
#else
  #include <kptyprocess.h>
  #include <kptydevice.h>
#endif

#include "result.h"

//NOTE: the \\s in the expressions is needed, because Maxima seems to sometimes insert newlines/spaces between the letters
//maybe this is caused by some behaviour if the Prompt is split into multiple "readStdout" calls
//the Expressions are encapsulated in () to allow capturing for the text
const QRegExp MaximaSession::MaximaOutputPrompt=QRegExp("(\\(\\s*%\\s*O\\s*[0-9\\s]*\\))"); //Text, maxima outputs, before any output


static QString initCmd=":lisp($load \"%1\")\n";

MaximaSession::MaximaSession( Cantor::Backend* backend ) : Session(backend)
{
    kDebug();
    m_initState=MaximaSession::NotInitialized;
    //m_maxima=0;
    m_process=0;
    m_justRestarted=false;
    m_useLegacy=false;

    m_variableModel=new MaximaVariableModel(this);
}

MaximaSession::~MaximaSession()
{
    kDebug();
}

void MaximaSession::login()
{
    kDebug()<<"login";
    if (m_process)
        m_process->deleteLater();
#ifndef Q_OS_WIN
    m_process=new KPtyProcess(this);
    m_process->setPtyChannels(KPtyProcess::StdinChannel|KPtyProcess::StdoutChannel);
    m_process->pty()->setEcho(false);
#else
    m_process=new KProcess(this);
    m_process->setOutputChannelMode(KProcess::SeparateChannels);
#endif

    m_process->setProgram(MaximaSettings::self()->path().toLocalFile());

    m_process->start();

    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(restartMaxima()));

#ifndef Q_OS_WIN
    connect(m_process->pty(), SIGNAL(readyRead()), this, SLOT(readStdOut()));
#else
    connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(readStdOut()));
#endif
    connect(m_process, SIGNAL(readyReadStandardError()), this, SLOT(readStdErr()));
    connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(reportProcessError(QProcess::ProcessError)));

    QString initFile=KStandardDirs::locate("data",   "cantor/maximabackend/cantor-initmaxima.lisp");
    kDebug()<<"initFile: "<<initFile;
    QString cmd=initCmd.arg(initFile);
    kDebug()<<"sending cmd: "<<cmd<<endl;

#ifndef Q_OS_WIN
    m_process->pty()->write(cmd.toUtf8());
#else
    m_process->write(cmd.toUtf8());
#endif

    Cantor::Expression* expr=evaluateExpression("print(____END_OF_INIT____);",
                                                Cantor::Expression::DeleteOnFinish);

    expr->setInternal(true);
    //check if we actually landed in the queue and there wasn't some
    //error beforehand instead
    if(m_expressionQueue.contains(dynamic_cast<MaximaExpression*>(expr)))
    {
        //move this expression to the front
        m_expressionQueue.prepend(m_expressionQueue.takeLast());
    }

    //reset the typesetting state
    setTypesettingEnabled(isTypesettingEnabled());


    m_initState=MaximaSession::Initializing;
    runFirstExpression();

}

void MaximaSession::logout()
{
    kDebug()<<"logout";

    if(!m_process)
        return;

    disconnect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(restartMaxima()));

    if(status()==Cantor::Session::Done)
    {
#ifndef Q_OS_WIN
        m_process->pty()->write("quit();\n");
#else
        m_process->write("quit();\n");
#endif

#ifdef Q_OS_WIN
        //Give maxima time to clean up
        kDebug()<<"waiting for maxima to finish";

        m_process->waitForFinished();
#endif
    }
    else
    {
        m_expressionQueue.clear();
    }

    //if it is still running, kill just kill it
    if(m_process->state()!=QProcess::NotRunning)
    {
        m_process->kill();
    }

    kDebug()<<"done logging out";

    delete m_process;
    m_process=0;

    kDebug()<<"destroyed maxima";

    m_expressionQueue.clear();
}

Cantor::Expression* MaximaSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave)
{
    kDebug()<<"evaluating: "<<cmd;
    MaximaExpression* expr=new MaximaExpression(this);
    expr->setFinishingBehavior(behave);
    expr->setCommand(cmd);

    expr->evaluate();

    return expr;
}


void MaximaSession::appendExpressionToQueue(MaximaExpression* expr)
{
    m_expressionQueue.append(expr);

    kDebug()<<"queue: "<<m_expressionQueue.size();
    if(m_expressionQueue.size()==1)
    {
        changeStatus(Cantor::Session::Running);
        runFirstExpression();
    }
}

void MaximaSession::readStdErr()
{
   kDebug()<<"reading stdErr";
   if (!m_process)
       return;
   QString out=m_process->readAllStandardError();

   if(m_expressionQueue.size()>0)
   {
       MaximaExpression* expr=m_expressionQueue.first();
       expr->parseError(out);
   }
}

void MaximaSession::readStdOut()
{
    kDebug()<<"reading stdOut";
    if (!m_process)
	return;
#ifndef Q_OS_WIN
    QString out=m_process->pty()->readAll();
#else
    QString out=m_process->readAllStandardOutput();
#endif

    kDebug()<<"out: "<<out;


    m_cache+=out;

    bool parsingSuccessful=true;

    if(m_expressionQueue.isEmpty())
    {
        kDebug()<<"got output without active expression. dropping: "<<endl
                <<m_cache;
        m_cache.clear();
        return;
    }

    MaximaExpression* expr=m_expressionQueue.first();

    if(expr)
        parsingSuccessful=expr->parseOutput(m_cache);
    else
        parsingSuccessful=false;

    if(parsingSuccessful)
    {
        kDebug()<<"parsing successful. dropping "<<m_cache;
        m_cache.clear();
    }

}

void MaximaSession::killLabels()
{
    Cantor::Expression* e=evaluateExpression("kill(labels);", Cantor::Expression::DeleteOnFinish);
    e->setInternal(true);
    connect(e, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SIGNAL(ready()));
}

void MaximaSession::reportProcessError(QProcess::ProcessError e)
{
    kDebug()<<"process error"<<e;
    if(e==QProcess::FailedToStart)
    {
        changeStatus(Cantor::Session::Done);
        emit error(i18n("Failed to start Maxima"));
    }
}

void MaximaSession::currentExpressionChangedStatus(Cantor::Expression::Status status)
{
    MaximaExpression* expression=m_expressionQueue.first();
    kDebug() << expression << status;

    if(m_initState==MaximaSession::Initializing
       && expression->command().contains( "____END_OF_INIT____"))
    {
        kDebug()<<"initialized";
        m_expressionQueue.removeFirst();

        m_initState=MaximaSession::Initialized;
        m_cache.clear();

        runFirstExpression();

        //QTimer::singleShot(0, this, SLOT(killLabels()));
        killLabels();

        changeStatus(Cantor::Session::Done);

        return;
    }


    if(status!=Cantor::Expression::Computing) //The session is ready for the next command
    {
        kDebug()<<"expression finished";
        disconnect(expression, SIGNAL(statusChanged(Cantor::Expression::Status)),
                   this, SLOT(currentExpressionChangedStatus(Cantor::Expression::Status)));

        kDebug()<<"running next command";

        m_expressionQueue.removeFirst();
        if(m_expressionQueue.isEmpty())
        {
            //if we are done with all the commands in the queue,
            //use the opportunity to update the variablemodel (if the last command wasn't already an update, as infinite loops aren't fun)
            QRegExp exp=QRegExp(QRegExp::escape(MaximaVariableModel::inspectCommand).arg("(values|functions)"));
            QRegExp exp2=QRegExp(QRegExp::escape(MaximaVariableModel::variableInspectCommand).arg("(values|functions)"));

            if(MaximaSettings::variableManagement()&&!exp.exactMatch(expression->command())&&!exp2.exactMatch(expression->command()))
            {
                m_variableModel->checkForNewFunctions();
                m_variableModel->checkForNewVariables();
            }else
            {
                changeStatus(Cantor::Session::Done);
            }

        }else
        {
            runFirstExpression();
        }
    }

}

void MaximaSession::runFirstExpression()
{
    if(m_initState==MaximaSession::NotInitialized)
    {
        kDebug()<<"not ready to run expression";
        return;

    }
    kDebug()<<"running next expression";
    if (!m_process)
	return;

    if(!m_expressionQueue.isEmpty())
    {
        MaximaExpression* expr=m_expressionQueue.first();
        QString command=expr->internalCommand();
        connect(expr, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SLOT(currentExpressionChangedStatus(Cantor::Expression::Status)));

        if(command.isEmpty())
        {
            kDebug()<<"empty command";
            expr->forceDone();
        }else
        {
            kDebug()<<"writing "<<command+'\n'<<" to the process";
            m_cache.clear();
            QString cmd=(command+'\n');
#ifndef Q_OS_WIN
            m_process->pty()->write(cmd.toUtf8());
#else
            m_process->write(cmd.toUtf8());
#endif
        }
    }
}

void MaximaSession::interrupt()
{
    if(!m_expressionQueue.isEmpty())
        m_expressionQueue.first()->interrupt();

    m_expressionQueue.clear();
    changeStatus(Cantor::Session::Done);
}

void MaximaSession::interrupt(MaximaExpression* expr)
{
    Q_ASSERT(!m_expressionQueue.isEmpty());

    if(expr==m_expressionQueue.first())
    {
        disconnect(expr, 0, this, 0);
        //TODO for non unix platforms sending signals probably won't work
        const int pid=m_process->pid();
        kill(pid, SIGINT);

        kDebug()<<"done interrupting";
    }else
    {
        m_expressionQueue.removeAll(expr);
    }
}

void MaximaSession::sendInputToProcess(const QString& input)
{
    kDebug()<<"WARNING: use this method only if you know what you're doing. Use evaluateExpression to run commands";
    kDebug()<<"running "<<input;

#ifndef Q_OS_WIN
            m_process->pty()->write(input.toUtf8());
#else
            m_process->write(input.toUtf8());
#endif
}

void MaximaSession::restartMaxima()
{
    kDebug()<<"restarting maxima cooldown: "<<m_justRestarted;

    if(!m_justRestarted)
    {
        //If maxima finished, before the session was initialized
        //We try to use Legacy commands for startups (Maxima <5.18)
        //In this case, don't require the cooldown
        if(m_initState!=MaximaSession::Initialized)
        {
            m_useLegacy=!m_useLegacy;
            kDebug()<<"Initializing maxima failed now trying legacy support: "<<m_useLegacy;
        }
        else
        {
             emit error(i18n("Maxima crashed. restarting..."));
             //remove the command that caused maxima to crash (to avoid infinite loops)
             if(!m_expressionQueue.isEmpty())
                 m_expressionQueue.removeFirst();

            m_justRestarted=true;
            QTimer::singleShot(1000, this, SLOT(restartsCooledDown()));
        }

        disconnect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(restartMaxima()));
        login();
    }else
    {
        if(!m_expressionQueue.isEmpty())
            m_expressionQueue.removeFirst();
        KMessageBox::error(0, i18n("Maxima crashed twice within a short time. Stopping to try starting"), i18n("Error - Cantor"));
    }
}

void MaximaSession::restartsCooledDown()
{
    kDebug()<<"maxima restart cooldown";
    m_justRestarted=false;
}


void MaximaSession::setTypesettingEnabled(bool enable)
{
    //we use the lisp command to set the variable, as those commands
    //don't mess with the labels and history
    const QString& val=(enable==true ? "t":"nil");
    Cantor::Expression* exp=evaluateExpression(QString(":lisp(setf $display2d %1)").arg(val), Cantor::Expression::DeleteOnFinish);
    exp->setInternal(true);

    Cantor::Session::setTypesettingEnabled(enable);
}

Cantor::CompletionObject* MaximaSession::completionFor(const QString& command, int index)
{
    return new MaximaCompletionObject(command, index, this);
}

Cantor::SyntaxHelpObject* MaximaSession::syntaxHelpFor(const QString& command)
{
    return new MaximaSyntaxHelpObject(command, this);
}

QSyntaxHighlighter* MaximaSession::syntaxHighlighter(QObject* parent)
{
    return new MaximaHighlighter(parent, this);
}

QAbstractItemModel* MaximaSession::variableModel()
{
    return m_variableModel;
}

#include "maximasession.moc"
