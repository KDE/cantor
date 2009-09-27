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

#include <kdebug.h>
#include <kprocess.h>
#include <signal.h>
#include "settings.h"

#include "result.h"


const QRegExp MaximaSession::MaximaPrompt=QRegExp("\\(%I[0-9]*\\)"); //Text, maxima outputs, if it's taking new input
const QRegExp MaximaSession::MaximaOutputPrompt=QRegExp("\\(%O[0-9]*\\)"); //Text, maxima outputs, before any output


static QByteArray initCmd="display2d:false$                     \n"\
                          "inchar:%I$                           \n"\
                          "outchar:%O$                          \n"\
                          "print(____END_OF_INIT____);          \n";
static QByteArray texInitCmd="simp: false$ \n";

MaximaSession::MaximaSession( Cantor::Backend* backend) : Session(backend)
{
    kDebug();
    m_isInitialized=false;
    m_process=0;
    m_texConvertProcess=0;
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

    m_process=new KProcess(this);
    m_process->setProgram(MaximaSettings::self()->path().toLocalFile());
    m_process->setOutputChannelMode(KProcess::SeparateChannels);
    //m_process->setPtyChannels(KProcess::AllChannels);
    //m_process->pty()->setEcho(false);
    //m_process->setUseUtmp(true);

    connect(m_process, SIGNAL(readyRead()), this, SLOT(readStdOut()));
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(restartMaxima()));
    m_process->start();
    m_process->write(initCmd);
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

    if(m_expressionQueue.size()==1)
    {
        changeStatus(Cantor::Session::Running);
        runFirstExpression();
    }
}

void MaximaSession::readStdOut()
{
//    if(!m_process->canReadLine()) return;

    kDebug()<<"reading stdOut";
    QString out=m_process->readAll();
    kDebug()<<"out: "<<out;


    if(out.contains("____END_OF_INIT____"))
    {
        kDebug()<<"initialized";
        out.remove("____END_OF_INIT____");

        m_isInitialized=true;

        runFirstExpression();
        evaluateExpression("kill(labels);", Cantor::Expression::DeleteOnFinish);


        changeStatus(Cantor::Session::Done);

        emit ready();
    }

    kDebug()<<"queuesize: "<<m_expressionQueue.size();
    if(m_isInitialized&&!m_expressionQueue.isEmpty())
    {
        MaximaExpression* expr=m_expressionQueue.first();
        expr->parseOutput(out);
    }
}

void MaximaSession::readTeX()
{
    kDebug()<<"reading stdOut";
    QString out=m_texConvertProcess->readAll();
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
    if(!m_expressionQueue.isEmpty()&&m_isInitialized)
    {
        MaximaExpression* expr=m_expressionQueue.first();
        connect(expr, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SLOT(currentExpressionChangedStatus(Cantor::Expression::Status)));
        QString command=expr->internalCommand();

        kDebug()<<"writing "<<command+'\n'<<" to the process";
        m_process->write((command+'\n').toLatin1());
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
            m_texConvertProcess->write(texCmd.toUtf8());
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

void MaximaSession::sendSignalToProcess(int signal)
{
    kDebug()<<"sending signal....."<<signal;
    kill(m_process->pid(), signal);
}

void MaximaSession::sendInputToProcess(const QString& input)
{
    kDebug()<<"WARNING: use this method only if you know what you're doing. Use evaluateExpression to run commands";
    kDebug()<<"running "<<input;
    m_process->write(input.toLatin1());
}

void MaximaSession::restartMaxima()
{
    kDebug()<<"restarting maxima";
    //remove the command that caused maxima to crash (to avoid infinite loops)
    if(!m_expressionQueue.isEmpty())
        m_expressionQueue.removeFirst();
    login();
}

void MaximaSession::startTexConvertProcess()
{
    //Start the process that is used to convert to LaTeX
    m_texConvertProcess=new KProcess(this);
    m_texConvertProcess->setProgram(MaximaSettings::self()->path().toLocalFile());
    m_texConvertProcess->setOutputChannelMode(KProcess::SeparateChannels);
    //m_texConvertProcess->setPtyChannels(KProcess::AllChannels);
    //m_texConvertProcess->pty()->setEcho(false);

    connect(m_texConvertProcess, SIGNAL(readyRead()), this, SLOT(readTeX()));
    connect(m_texConvertProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(startTexConvertProcess()));
    m_texConvertProcess->start();
    m_texConvertProcess->write(texInitCmd);
    m_texConvertProcess->write(initCmd);
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

#include "maximasession.moc"
