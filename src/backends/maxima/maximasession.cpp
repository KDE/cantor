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
    Copyright (C) 2017-2018 Alexander Semke (alexander.semke@web.de)
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
#include <QProcess>
#include <QTimer>

#include <KMessageBox>
#include <KLocalizedString>

#ifndef Q_OS_WIN
#include <signal.h>
#endif


//NOTE: the \\s in the expressions is needed, because Maxima seems to sometimes insert newlines/spaces between the letters
//maybe this is caused by some behaviour if the Prompt is split into multiple "readStdout" calls
//the Expressions are encapsulated in () to allow capturing for the text
const QRegExp MaximaSession::MaximaOutputPrompt=QRegExp(QLatin1String("(\\(\\s*%\\s*o\\s*[0-9\\s]*\\))")); //Text, maxima outputs, before any output


MaximaSession::MaximaSession( Cantor::Backend* backend ) : Session(backend),
    m_process(nullptr),
    m_variableModel(new MaximaVariableModel(this)),
    m_justRestarted(false)
{
}

void MaximaSession::login()
{
    qDebug()<<"login";

    if (m_process)
        return; //TODO: why do we call login() again?!?

    emit loginStarted();
    QStringList arguments;
    arguments << QLatin1String("--quiet"); //Suppress Maxima start-up message
    const QString initFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("cantor/maximabackend/cantor-initmaxima.lisp"));
    arguments << QLatin1String("--init-lisp=") + initFile; //Set the name of the Lisp initialization file

    m_process = new QProcess(this);
    m_process->start(MaximaSettings::self()->path().toLocalFile(), arguments);
    m_process->waitForStarted();
    m_process->waitForReadyRead();
    qDebug()<<m_process->readAllStandardOutput();

    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(restartMaxima()));
    connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(readStdOut()));
    connect(m_process, SIGNAL(readyReadStandardError()), this, SLOT(readStdErr()));
    connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(reportProcessError(QProcess::ProcessError)));

    //TODO
//     if(!MaximaSettings::self()->autorunScripts().isEmpty()){
//         QString autorunScripts = MaximaSettings::self()->autorunScripts().join(QLatin1String("\n"));
//         evaluateExpression(autorunScripts, MaximaExpression::DeleteOnFinish);
// //         runFirstExpression();
//     }

    changeStatus(Session::Done);
    emit loginDone();
    qDebug()<<"login done";
}

void MaximaSession::logout()
{
    qDebug()<<"logout";

    if(!m_process)
        return;

    disconnect(m_process, nullptr, this, nullptr);

//     if(status()==Cantor::Session::Running)
        //TODO: terminate the running expressions first

    write(QLatin1String("quit();\n"));
    qDebug()<<"waiting for maxima to finish";
    m_process->waitForFinished();
    qDebug()<<"maxima exit finished";

    if(m_process->state() != QProcess::NotRunning)
    {
        m_process->kill();
        qDebug()<<"maxima still running, process kill enforced";
    }

    expressionQueue().clear();
    delete m_process;
    m_process = nullptr;

    changeStatus(Status::Disable);

    qDebug()<<"logout done";
}

Cantor::Expression* MaximaSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave, bool internal)
{
    qDebug() << "evaluating: " << cmd;
    MaximaExpression* expr = new MaximaExpression(this, internal);
    expr->setFinishingBehavior(behave);
    expr->setCommand(cmd);
    expr->evaluate();

    return expr;
}

void MaximaSession::readStdErr()
{
   qDebug()<<"reading stdErr";
   if (!m_process)
       return;
   QString out=QLatin1String(m_process->readAllStandardError());

   if(expressionQueue().size()>0)
   {
       MaximaExpression* expr = static_cast<MaximaExpression*>(expressionQueue().first());
       expr->parseError(out);
   }
}

void MaximaSession::readStdOut()
{
    QString out = QLatin1String(m_process->readAllStandardOutput());
    m_cache += out;

    //collect the multi-line output until Maxima has finished the calculation and returns a new promt
    if ( !out.contains(QLatin1String("</cantor-prompt>")) )
        return;

    if(expressionQueue().isEmpty())
    {
        //queue is empty, interrupt was called, nothing to do here
        qDebug()<<m_cache;
        m_cache.clear();
        return;
    }

    MaximaExpression* expr = static_cast<MaximaExpression*>(expressionQueue().first());
    if (!expr)
        return; //should never happen

    qDebug()<<"output: " << m_cache;
    expr->parseOutput(m_cache);
    m_cache.clear();
}

void MaximaSession::killLabels()
{
    Cantor::Expression* e=evaluateExpression(QLatin1String("kill(labels);"), Cantor::Expression::DeleteOnFinish, true);
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
    Cantor::Expression* expression = expressionQueue().first();
    qDebug() << "expression status changed: command = " << expression->command() << ", status = " << status;

    if(status!=Cantor::Expression::Computing) //The session is ready for the next command
    {
        qDebug()<<"################################## EXPRESSION END ###############################################";
        disconnect(expression, SIGNAL(statusChanged(Cantor::Expression::Status)),
                   this, SLOT(currentExpressionChangedStatus(Cantor::Expression::Status)));

        expressionQueue().removeFirst();
        if(expressionQueue().isEmpty())
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
    qDebug()<<"running next expression";
    if (!m_process)
        return;

    if(!expressionQueue().isEmpty())
    {
        MaximaExpression* expr = static_cast<MaximaExpression*>(expressionQueue().first());
        QString command=expr->internalCommand();
        connect(expr, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SLOT(currentExpressionChangedStatus(Cantor::Expression::Status)));

        expr->setStatus(Cantor::Expression::Computing);
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
    if(!expressionQueue().isEmpty())
    {
        qDebug()<<"interrupting " << expressionQueue().first()->command();
        if(m_process->state() != QProcess::NotRunning)
        {
#ifndef Q_OS_WIN
            const int pid=m_process->pid();
            kill(pid, SIGINT);
#else
            ; //TODO: interrupt the process on windows
#endif
        }
        expressionQueue().first()->interrupt();
        expressionQueue().removeFirst();
        foreach (Cantor::Expression* expression, expressionQueue())
            expression->setStatus(Cantor::Expression::Done);
        expressionQueue().clear();

        qDebug()<<"done interrupting";
    }

    changeStatus(Cantor::Session::Done);
    m_cache.clear();
}

void MaximaSession::sendInputToProcess(const QString& input)
{
    write(input);
}

void MaximaSession::restartMaxima()
{
    qDebug()<<"restarting maxima cooldown: "<<m_justRestarted;

    if(!m_justRestarted)
    {
        emit error(i18n("Maxima crashed. restarting..."));
        //remove the command that caused maxima to crash (to avoid infinite loops)
        if(!expressionQueue().isEmpty())
            expressionQueue().removeFirst();

        m_justRestarted=true;
        QTimer::singleShot(1000, this, SLOT(restartsCooledDown()));

        disconnect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(restartMaxima()));
        login();
    }else
    {
        if(!expressionQueue().isEmpty())
            expressionQueue().removeFirst();
        KMessageBox::error(nullptr, i18n("Maxima crashed twice within a short time. Stopping to try starting"), i18n("Error - Cantor"));
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
    Cantor::Expression* exp=evaluateExpression(QString::fromLatin1(":lisp(setf $display2d %1)").arg(val), Cantor::Expression::DeleteOnFinish, true);

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
    qDebug()<<"################################## EXPRESSION START ###############################################";
    qDebug()<<"sending expression to maxima process: " << exp;
    m_process->write(exp.toUtf8());
}
