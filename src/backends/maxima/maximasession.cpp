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
#include "result.h"
#include "settings.h"

#include <QDebug>
#include <QTimer>
#include <KMessageBox>
#include <KLocalizedString>
#include <signal.h>

#ifdef Q_OS_WIN
  #include <KProcess>
#else
  #include <KPtyProcess>
  #include <KPtyDevice>
#endif


//NOTE: the \\s in the expressions is needed, because Maxima seems to sometimes insert newlines/spaces between the letters
//maybe this is caused by some behaviour if the Prompt is split into multiple "readStdout" calls
//the Expressions are encapsulated in () to allow capturing for the text
const QRegExp MaximaSession::MaximaOutputPrompt=QRegExp(QLatin1String("(\\(\\s*%\\s*O\\s*[0-9\\s]*\\))")); //Text, maxima outputs, before any output

static QString initCmd=QLatin1String(":lisp($load \"%1\")");

MaximaSession::MaximaSession( Cantor::Backend* backend ) : Session(backend),
    m_initState(MaximaSession::NotInitialized),
    m_process(0),
    m_justRestarted(false),
    m_variableModel(new MaximaVariableModel(this))
{
}

MaximaSession::~MaximaSession()
{
}

void MaximaSession::login()
{
    qDebug()<<"login";
    emit loginStarted();

    if (m_process)
        m_process->deleteLater();

#ifndef Q_OS_WIN
    m_process=new KPtyProcess(this);
    m_process->setPtyChannels(KPtyProcess::StdinChannel|KPtyProcess::StdoutChannel);
    m_process->pty()->setEcho(false);
    connect(m_process->pty(), SIGNAL(readyRead()), this, SLOT(readStdOut()));
#else
    m_process=new KProcess(this);
    m_process->setOutputChannelMode(KProcess::SeparateChannels);
    connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(readStdOut()));
#endif

    m_process->setProgram(MaximaSettings::self()->path().toLocalFile());
    m_process->start();

    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(restartMaxima()));
    connect(m_process, SIGNAL(readyReadStandardError()), this, SLOT(readStdErr()));
    connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(reportProcessError(QProcess::ProcessError)));

    const QString initFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("cantor/maximabackend/cantor-initmaxima.lisp"));
    write(initCmd.arg(initFile));

    Cantor::Expression* expr=evaluateExpression(QLatin1String("print(____END_OF_INIT____);"),
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

    if(!MaximaSettings::self()->autorunScripts().isEmpty()){
        QString autorunScripts = MaximaSettings::self()->autorunScripts().join(QLatin1String("\n"));
        evaluateExpression(autorunScripts, MaximaExpression::DeleteOnFinish);
    }

    runFirstExpression();

    emit loginDone();
}

void MaximaSession::logout()
{
    qDebug()<<"logout";

    if(!m_process)
        return;

    disconnect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(restartMaxima()));

    if(status()==Cantor::Session::Done)
    {
        write(QLatin1String("quit();\n"));

#ifdef Q_OS_WIN
        //Give maxima time to clean up
        qDebug()<<"waiting for maxima to finish";

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

    qDebug()<<"done logging out";

    delete m_process;
    m_process=0;

    qDebug()<<"destroyed maxima";

    m_expressionQueue.clear();
}

Cantor::Expression* MaximaSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave)
{
    MaximaExpression* expr = new MaximaExpression(this);
    expr->setFinishingBehavior(behave);
    expr->setCommand(cmd);
    expr->evaluate();

    return expr;
}

void MaximaSession::appendExpressionToQueue(MaximaExpression* expr)
{
    m_expressionQueue.append(expr);

    qDebug()<<"queue: "<<m_expressionQueue.size();
    if(m_expressionQueue.size()==1)
    {
        changeStatus(Cantor::Session::Running);
        runFirstExpression();
    }
}

void MaximaSession::readStdErr()
{
   qDebug()<<"reading stdErr";
   if (!m_process)
       return;
   QString out=QLatin1String(m_process->readAllStandardError());

   if(m_expressionQueue.size()>0)
   {
       MaximaExpression* expr=m_expressionQueue.first();
       expr->parseError(out);
   }
}

void MaximaSession::readStdOut()
{
    qDebug()<<"reading stdOut";
    if (!m_process)
        return;
#ifndef Q_OS_WIN
    QString out=QLatin1String(m_process->pty()->readAll());
#else
    QString out=m_process->readAllStandardOutput();
#endif

    out.remove(QLatin1Char('\r'));

    qDebug()<<"out: "<<out;

    m_cache+=out;

    bool parsingSuccessful=true;

    if(m_expressionQueue.isEmpty())
    {
        qDebug()<<"got output without active expression. dropping: "<<endl
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
        qDebug()<<"parsing successful. dropping "<<m_cache;
        m_cache.clear();
    }

}

void MaximaSession::killLabels()
{
    Cantor::Expression* e=evaluateExpression(QLatin1String("kill(labels);"), Cantor::Expression::DeleteOnFinish);
    e->setInternal(true);
    //TODO: what for? connect(e, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SIGNAL(ready()));
}

void MaximaSession::reportProcessError(QProcess::ProcessError e)
{
    qDebug()<<"process error"<<e;
    if(e==QProcess::FailedToStart)
    {
        changeStatus(Cantor::Session::Done);
        emit error(i18n("Failed to start Maxima"));
    }
}

void MaximaSession::currentExpressionChangedStatus(Cantor::Expression::Status status)
{
    MaximaExpression* expression=m_expressionQueue.first();
    qDebug() << expression << status;

    if(m_initState==MaximaSession::Initializing
       && expression->command().contains( QLatin1String("____END_OF_INIT____")))
    {
        qDebug()<<"initialized";
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
        qDebug()<<"expression finished";
        disconnect(expression, SIGNAL(statusChanged(Cantor::Expression::Status)),
                   this, SLOT(currentExpressionChangedStatus(Cantor::Expression::Status)));

        qDebug()<<"running next command";

        m_expressionQueue.removeFirst();
        if(m_expressionQueue.isEmpty())
        {
            //if we are done with all the commands in the queue,
            //use the opportunity to update the variablemodel (if the last command wasn't already an update, as infinite loops aren't fun)
            QRegExp exp=QRegExp(QRegExp::escape(MaximaVariableModel::inspectCommand).arg(QLatin1String("(values|functions)")));
            QRegExp exp2=QRegExp(QRegExp::escape(MaximaVariableModel::variableInspectCommand).arg(QLatin1String("(values|functions)")));

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
        qDebug()<<"not ready to run expression";
        return;

    }
    qDebug()<<"running next expression";
    if (!m_process)
        return;

    if(!m_expressionQueue.isEmpty())
    {
        MaximaExpression* expr=m_expressionQueue.first();
        QString command=expr->internalCommand();
        connect(expr, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SLOT(currentExpressionChangedStatus(Cantor::Expression::Status)));

        if(command.isEmpty())
        {
            qDebug()<<"empty command";
            expr->forceDone();
        }
        else
        {
            m_cache.clear();
            write(command + QLatin1Char('\n'));
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

        qDebug()<<"done interrupting";
    }else
    {
        m_expressionQueue.removeAll(expr);
    }
}

void MaximaSession::sendInputToProcess(const QString& input)
{
    qDebug()<<"WARNING: use this method only if you know what you're doing. Use evaluateExpression to run commands";
    qDebug()<<"running "<<input;
    write(input);
}

void MaximaSession::restartMaxima()
{
    qDebug()<<"restarting maxima cooldown: "<<m_justRestarted;

    if(!m_justRestarted)
    {
        emit error(i18n("Maxima crashed. restarting..."));
        //remove the command that caused maxima to crash (to avoid infinite loops)
        if(!m_expressionQueue.isEmpty())
            m_expressionQueue.removeFirst();

        m_justRestarted=true;
        QTimer::singleShot(1000, this, SLOT(restartsCooledDown()));

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
    qDebug()<<"maxima restart cooldown";
    m_justRestarted=false;
}

void MaximaSession::setTypesettingEnabled(bool enable)
{
    //we use the lisp command to set the variable, as those commands
    //don't mess with the labels and history
    const QString& val=QLatin1String((enable==true ? "t":"nil"));
    Cantor::Expression* exp=evaluateExpression(QString::fromLatin1(":lisp(setf $display2d %1)").arg(val), Cantor::Expression::DeleteOnFinish);
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

void MaximaSession::write(const QString& exp) {
    qDebug()<<"sending expression to maxima process: " << exp << endl;
#ifndef Q_OS_WIN
    m_process->pty()->write(exp.toUtf8());
#else
    m_process->write(exp.toUtf8());
#endif
}

#include "maximasession.moc"
