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
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
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
#include <kprocess.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <signal.h>
#include "settings.h"

#include "result.h"

//NOTE: the \\s in the expressions is needed, because Maxima seems to sometimes insert newlines/spaces between the letters
//maybe this is caused by some behaviour if the Prompt is split into multiple "readStdout" calls
//the Expressions are encapsulated in () to allow capturing for the text
const QRegExp MaximaSession::MaximaPrompt=QRegExp("(<prompt>\\s<prompt>)"); //Text, maxima outputs, if it's taking new input
const QRegExp MaximaSession::MaximaOutputPrompt=QRegExp("(\\(\\s*%\\s*O\\s*[0-9\\s]*\\))"); //Text, maxima outputs, before any output


static QString initCmd=":lisp($load \"%1\")";

MaximaSession::MaximaSession( Cantor::Backend* backend ) : Session(backend)
{
    kDebug();
    m_initState=MaximaSession::NotInitialized;
    m_server=0;
    m_maxima=0;
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
    if(!m_server||!m_server->isListening())
        startServer();

    m_maxima=0;
    m_process=new KProcess(this);
    QStringList args;
    //TODO: these parameters may need tweaking to run on windows (see wxmaxima for hints)
    if(m_useLegacy)
        args<<"-r"<<QString(":lisp (setup-server %1)").arg(m_server->serverPort());
    else
        args<<"-r"<<QString(":lisp (setup-client %1)").arg(m_server->serverPort());

    m_process->setProgram(MaximaSettings::self()->path().toLocalFile(),args);

    m_process->start();

    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(restartMaxima()));
    connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(reportProcessError(QProcess::ProcessError)));

}

void MaximaSession::startServer()
{
    kDebug()<<"starting up maxima server";
    const int defaultPort=4060;
    int port=defaultPort;
    m_server=new QTcpServer(this);
    connect(m_server, SIGNAL(newConnection()), this, SLOT(newConnection()));

    while(! m_server->listen(QHostAddress::LocalHost, port) )
    {
        kDebug()<<"Could not listen to "<<port;
        port++;
        kDebug()<<"Now trying "<<port;

        if(port>defaultPort+50)
        {
            KMessageBox::error(0, i18n("Could not start the server."), i18n("Error - Cantor"));
            return;
        }
    }

    kDebug()<<"got a server on "<<port;
}

void MaximaSession::newMaximaClient(QTcpSocket* socket)
{
    kDebug()<<"got new maxima client";
    m_maxima=socket;
    connect(m_maxima, SIGNAL(readyRead()), this, SLOT(readStdOut()));

    QString initFile=KStandardDirs::locate("data",   "cantor/maximabackend/cantor-initmaxima.lisp");
    kDebug()<<"initFile: "<<initFile;
    QString cmd=initCmd.arg(initFile);
    kDebug()<<"sending cmd: "<<cmd<<endl;

    m_maxima->write(cmd.toLatin1());

    Cantor::Expression* expr=evaluateExpression("print(____END_OF_INIT____);",
                                                Cantor::Expression::DeleteOnFinish);

    //move this expression to the front
    m_expressionQueue.prepend(m_expressionQueue.takeLast());

    //reset the typesetting state
    setTypesettingEnabled(isTypesettingEnabled());


    m_initState=MaximaSession::Initializing;
    runFirstExpression();
}

void MaximaSession::logout()
{
    kDebug()<<"logout";

    if(!m_process||!m_maxima)
        return;

    disconnect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(restartMaxima()));

    if(m_expressionQueue.isEmpty())
    {
        m_maxima->write("quit();\n");
        m_maxima->flush();
        //evaluateExpression("quit();", Cantor::Expression::DeleteOnFinish);
    }
    else
    {
        m_expressionQueue.clear();
    }

    //Give maxima time to clean up
    kDebug()<<"waiting for maxima to finish";

    if(m_process->state()!=QProcess::NotRunning)
    {
        if(!m_maxima->waitForDisconnected(3000))
        {
            m_process->kill();
            m_maxima->waitForDisconnected(3000);
        }
    }

    m_maxima->close();

    kDebug()<<"done logging out";

    delete m_process;
    m_process=0;
    delete m_maxima;
    m_maxima=0;

    kDebug()<<"destroyed maxima";

    m_expressionQueue.clear();
}

void MaximaSession::newConnection()
{
    kDebug()<<"new connection";
    QTcpSocket* const socket=m_server->nextPendingConnection();

    if(m_maxima!=0)
        kDebug()<<"WARNING: got a new client without needing one";

    newMaximaClient(socket);
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

void MaximaSession::readStdOut()
{
    kDebug()<<"reading stdOut";
    if (!m_maxima)
	return;
    QString out=m_maxima->readAll();
    kDebug()<<"out: "<<out;


    m_cache+=out;

    /*
    if(m_cache.contains(QRegExp(QString("%1 %2").arg(MaximaOutputPrompt.pattern()).arg("____END_OF_INIT____")))&&m_cache.contains("</prompt>"))
    {

    }
    */

    //if(!m_initState==MaximaExpression::Initialized)
    //    return;


    bool parsingSuccessfull=true;
    while(!m_cache.isEmpty()&&parsingSuccessfull)
    {
        if(m_expressionQueue.isEmpty())
        {
            kDebug()<<"got output without active expression. dropping: "<<endl
                    <<m_cache;
            m_cache.clear();
            break;
        }

        MaximaExpression* expr=m_expressionQueue.first();
        if(expr)
            parsingSuccessfull=expr->parseOutput(m_cache);
        else
            parsingSuccessfull=false;

    }
}


void MaximaSession::killLabels()
{
    Cantor::Expression* e=evaluateExpression("kill(labels);", Cantor::Expression::DeleteOnFinish);
    connect(e, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SIGNAL(ready()));
}

void MaximaSession::reportProcessError(QProcess::ProcessError e)
{
    kDebug()<<"process error";
    if(e==QProcess::FailedToStart)
    {
        changeStatus(Cantor::Session::Done);
        emit error(i18n("Failed to start Maxima"));
    }
}

void MaximaSession::currentExpressionChangedStatus(Cantor::Expression::Status status)
{
    MaximaExpression* expression=m_expressionQueue.first();

    if(m_initState==MaximaSession::Initializing
       && expression->command().contains( "____END_OF_INIT____"))
    {
        kDebug()<<"initialized";

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
        MaximaExpression* expression=m_expressionQueue.first();
        disconnect(expression, SIGNAL(statusChanged(Cantor::Expression::Status)),
                   this, SLOT(currentExpressionChangedStatus(Cantor::Expression::Status)));

        kDebug()<<"running next command";
        m_expressionQueue.removeFirst();
        if(m_expressionQueue.isEmpty())
        {
            //if we are done with all the commands in the queue,
            //use the opportinity to update the variablemodel (if the last command wasn't already an update, as infinite loops aren't fun)
            QRegExp exp=QRegExp(QRegExp::escape(MaximaVariableModel::inspectCommand).arg("(values|functions)"));
            QRegExp exp2=QRegExp(QRegExp::escape(MaximaVariableModel::variableInspectCommand).arg("(values|functions)"));
            if(expression->status()==Cantor::Expression::Done&&!exp.exactMatch(expression->command())&&!exp2.exactMatch(expression->command()))
            {
                m_variableModel->checkForNewFunctions();
                m_variableModel->checkForNewVariables();
            }else
            {
                changeStatus(Cantor::Session::Done);
            }

        }
        runFirstExpression();
    }

}

void MaximaSession::runFirstExpression()
{
    if(!m_maxima||m_initState==MaximaSession::NotInitialized)
    {
        kDebug()<<"not ready to run expression";
        return;

    }
    kDebug()<<"running next expression";
    if (!m_maxima)
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
            m_maxima->write((command+'\n').toLatin1());
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
        disconnect(m_maxima, 0);
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
    m_maxima->write(input.toLatin1());
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
    evaluateExpression(QString(":lisp(setf $display2d %1)").arg(val), Cantor::Expression::DeleteOnFinish);

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
