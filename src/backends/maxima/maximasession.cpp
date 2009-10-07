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
#include "maximatabcompletionobject.h"

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
const QRegExp MaximaSession::MaximaPrompt=QRegExp("(\\(\\s*%\\s*I\\s*[0-9\\s]*\\))"); //Text, maxima outputs, if it's taking new input
const QRegExp MaximaSession::MaximaOutputPrompt=QRegExp("(\\(\\s*%\\s*O\\s*[0-9\\s]*\\))"); //Text, maxima outputs, before any output


static QByteArray initCmd="display2d:false$                     \n"\
                          "inchar:%I$                           \n"\
                          "outchar:%O$                          \n"\
                          "print(____END_OF_INIT____);          \n";
static QByteArray texInitCmd="simp: false$ \n";

MaximaSession::MaximaSession( Cantor::Backend* backend) : Session(backend)
{
    kDebug();
    m_isInitialized=false;
    m_server=0;
    m_maxima=0;
    m_process=0;
    m_texConvertProcess=0;
    m_texMaxima=0;
    m_justRestarted=false;
    m_useLegacy=false;
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
    m_maxima->write(initCmd);
}

void MaximaSession::newTexClient(QTcpSocket* socket)
{
    kDebug()<<"got new tex client";
    m_texMaxima=socket;

    connect(m_texMaxima, SIGNAL(readyRead()), this, SLOT(readTeX()));

    m_texMaxima->write(texInitCmd);
    m_texMaxima->write(initCmd);
}

void MaximaSession::logout()
{
    kDebug()<<"logout";
    disconnect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(restartMaxima()));

    if(m_expressionQueue.isEmpty())
        evaluateExpression("quit();", Cantor::Expression::DeleteOnFinish);
    else
        interrupt();
    //Give maxima time to clean up
    if(!m_process->waitForFinished(3000))
    {
        m_process->kill();
    }

    m_process->deleteLater();
    m_expressionQueue.clear();
}

void MaximaSession::newConnection()
{
    QTcpSocket* const socket=m_server->nextPendingConnection();
    if(m_maxima==0)
    {
        newMaximaClient(socket);
    }else if (m_texMaxima==0)
    {
        newTexClient(socket);
    }else
    {
        kDebug()<<"got another client, without needing one";
    }
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
    QString out=m_maxima->readAll();
    kDebug()<<"out: "<<out;


    if(out.contains(QRegExp(QString("%1 %2").arg(MaximaOutputPrompt.pattern()).arg("____END_OF_INIT____"))))
    {
        kDebug()<<"initialized";
        out.remove("____END_OF_INIT____");

        m_isInitialized=true;
        m_cache.clear();
        runFirstExpression();

        QTimer::singleShot(0, this, SLOT(killLabels()));

        return;
    }

    if(!m_isInitialized)
        return;

    m_cache+=out;

    if(m_cache.contains('\n')||m_cache.contains(MaximaPrompt))
    {
        kDebug()<<"letting parse"<<m_cache;
        letExpressionParseOutput();
    }
}

void MaximaSession::letExpressionParseOutput()
{
    kDebug()<<"queuesize: "<<m_expressionQueue.size();
    if(m_isInitialized&&!m_expressionQueue.isEmpty())
    {
        MaximaExpression* expr=m_expressionQueue.first();

        //send over the part of the cache to the last newline or last InputPrompt, whatever comes last
        const int index=m_cache.lastIndexOf('\n')+1;
        const int index2=MaximaPrompt.lastIndexIn(m_cache)+MaximaPrompt.matchedLength();
        const int max=qMax(index, index2);
        QString txt=m_cache.left(max);
        expr->parseOutput(txt);
        m_cache.remove(0, max);
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

void MaximaSession::readTeX()
{
    kDebug()<<"reading stdOut";
    QString out=m_texMaxima->readAll();
    kDebug()<<"out: "<<out;

    kDebug()<<"queuesize: "<<m_texQueue.size();
    if(!m_texQueue.isEmpty())
    {
        MaximaExpression* expr=m_texQueue.first();
        kDebug()<<"needs latex?: "<<expr->needsLatexResult();

        expr->parseTexResult(out);

        if(!expr->needsLatexResult())
        {
            kDebug()<<"expression doesn't need latex anymore";
            m_texQueue.removeFirst();
            runNextTexCommand();
        }
    }
}

void MaximaSession::currentExpressionChangedStatus(Cantor::Expression::Status status)
{
    if(status!=Cantor::Expression::Computing) //The session is ready for the next command
    {
        kDebug()<<"expression finished";
        MaximaExpression* expression=m_expressionQueue.first();
        disconnect(expression, SIGNAL(statusChanged(Cantor::Expression::Status)),
                   this, SLOT(currentExpressionChangedStatus(Cantor::Expression::Status)));

        if(expression->needsLatexResult())
        {
            kDebug()<<"asking for tex version";
            m_texQueue<<expression;
            if(m_texQueue.size()==1) //It only contains the actual item. start processing it
                runNextTexCommand();
        }

        kDebug()<<"running next command";
        m_expressionQueue.removeFirst();
        if(m_expressionQueue.isEmpty())
            changeStatus(Cantor::Session::Done);
        runFirstExpression();

    }

}

void MaximaSession::runFirstExpression()
{
    kDebug()<<"running next expression";

    if(m_isInitialized&&!m_expressionQueue.isEmpty())
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

void MaximaSession::runNextTexCommand()
{
    if(!m_texQueue.isEmpty())
    {
        MaximaExpression* expr=m_texQueue.first();
        QString cmd=expr->result()->data().toString().trimmed();

        //check if the result already is tex, by checking if the whole
        //text is contained within a $$ pair
        //(the additional signs in the regex ignore newlines and whitespaces
        // at the begin and the end).
        //if it is, drop the Tex command
        if(QRegExp("^\\s*\\$\\$.*\\$\\$\\s*$").exactMatch(cmd))
            m_texQueue.takeFirst();

        if(!cmd.isEmpty())
        {
            QStringList cmdParts=cmd.split(QChar::ParagraphSeparator);
            QString texCmd;
            foreach(const QString& part, cmdParts)
            {
                if(part.isEmpty())
                    continue;
                kDebug()<<"running "<<QString("tex(%1);").arg(part);
                texCmd+=QString("tex(%1);").arg(part);
            }
            texCmd+='\n';
            m_texMaxima->write(texCmd.toUtf8());
        }else
        {
            kDebug()<<"current tex request is empty, so drop it";
            m_texQueue.removeFirst();
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
        restartMaxima();
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
        if(!m_isInitialized)
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
        KMessageBox::error(0, i18n("Maxima crashed twice within a short time. Stopping to try starting"), i18n("Error - Cantor"));
    }
}

void MaximaSession::restartsCooledDown()
{
    kDebug()<<"maxima restart cooldown";
    m_justRestarted=false;
}

void MaximaSession::startTexConvertProcess()
{
    m_texMaxima=0;
    //Start the process that is used to convert to LaTeX
    m_texConvertProcess=new KProcess(this);
    QStringList args;
    if(m_useLegacy)
        args<<"-r"<<QString(":lisp (setup-server %1)").arg(m_server->serverPort());
    else
        args<<"-r"<<QString(":lisp (setup-client %1)").arg(m_server->serverPort());

    m_texConvertProcess->setProgram(MaximaSettings::self()->path().toLocalFile(),args);

    connect(m_texConvertProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(startTexConvertProcess()));
    m_texConvertProcess->start();
}

void MaximaSession::setTypesettingEnabled(bool enable)
{
    if(enable)
    {
        startTexConvertProcess();
        //LaTeX and Display2d don't go together and even deliver wrong results
        evaluateExpression("display2d:false", Cantor::Expression::DeleteOnFinish);
    }
    else if(m_texConvertProcess)
    {
        disconnect(m_texConvertProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(startTexConvertProcess()));
        m_texConvertProcess->deleteLater();
    }
    Cantor::Session::setTypesettingEnabled(enable);
}

Cantor::TabCompletionObject* MaximaSession::tabCompletionFor(const QString& command)
{
    return new MaximaTabCompletionObject(command, this);
}

#include "maximasession.moc"
