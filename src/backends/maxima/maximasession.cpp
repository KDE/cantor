/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009-2012 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2017-2022 Alexander Semke (alexander.semke@web.de)
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
#include <QStandardPaths>

#include <KMessageBox>
#include <KLocalizedString>

#ifndef Q_OS_WIN
#include <signal.h>
#endif


//NOTE: the \\s in the expressions is needed, because Maxima seems to sometimes insert newlines/spaces between the letters
//maybe this is caused by some behaviour if the Prompt is split into multiple "readStdout" calls
//the Expressions are encapsulated in () to allow capturing for the text
const QRegularExpression MaximaSession::MaximaOutputPrompt =
            QRegularExpression(QStringLiteral("(\\(\\s*%\\s*o\\s*[0-9\\s]*\\))")); //Text, maxima outputs, before any output
const QRegularExpression MaximaSession::MaximaInputPrompt =
            QRegularExpression(QStringLiteral("(\\(\\s*%\\s*i\\s*[0-9\\s]*\\))"));


MaximaSession::MaximaSession( Cantor::Backend* backend ) : Session(backend)
{
    setVariableModel(new MaximaVariableModel(this));
}

void MaximaSession::login()
{
    qDebug()<<"login";

    if (m_process)
        return; //TODO: why do we call login() again?!?

    emit loginStarted();
    QStringList arguments;
    arguments << QLatin1String("--quiet"); //Suppress Maxima start-up message
    const QString initFile = locateCantorFile(QLatin1String("maximabackend/cantor-initmaxima.lisp"));
    arguments << QLatin1String("--init-lisp=") + initFile; //Set the name of the Lisp initialization file

    // start the process
    m_process = new QProcess(this);
    m_process->start(MaximaSettings::self()->path().toLocalFile(), arguments);
    if (!m_process->waitForStarted())
    {
        changeStatus(Session::Disable);
        emit error(i18n("Failed to start Maxima, please check Maxima's installation."));
        emit loginDone();
        delete m_process;
        m_process = nullptr;
        return;
    }

    // Wait until first maxima prompt
    QString input;
    while (!input.contains(QLatin1String("</cantor-prompt>")))
    {
        if (!m_process->waitForReadyRead())
            break;
        input += QString::fromLatin1(m_process->readAllStandardOutput());
    }

    if (input.isEmpty())
    {
        changeStatus(Session::Disable);
        emit error(i18n("Maxima didn't respond with the proper prompt, please check Maxima's installation."));
        emit loginDone();
        delete m_process;
        m_process = nullptr;
        return;
    }

    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(restartMaxima()));
    connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(readStdOut()));
    connect(m_process, SIGNAL(readyReadStandardError()), this, SLOT(readStdErr()));
    connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(reportProcessError(QProcess::ProcessError)));

    //enable latex typesetting if needed
    const QString& val = QLatin1String((isTypesettingEnabled() ? "t":"nil"));
    evaluateExpression(QString::fromLatin1(":lisp(setf $display2d %1)").arg(val), Cantor::Expression::DeleteOnFinish, true);

    //auto-run scripts
    if(!MaximaSettings::self()->autorunScripts().isEmpty()){
        QString autorunScripts = MaximaSettings::self()->autorunScripts().join(QLatin1String(";"));
        autorunScripts.append(QLatin1String(";kill(labels)")); // Reset labels after running autorun scripts
        evaluateExpression(autorunScripts, MaximaExpression::DeleteOnFinish, true);
        updateVariables();
    }

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

    if (status() == Cantor::Session::Running)
        interrupt();

    write(QLatin1String("quit();\n"));
    qDebug()<<"waiting for maxima to finish";

    if(!m_process->waitForFinished(1000))
    {
        m_process->kill();
        qDebug()<<"maxima still running, process kill enforced";
    }
    m_process->deleteLater();
    m_process = nullptr;

    Session::logout();

    qDebug()<<"logout done";
}

MaximaSession::Mode MaximaSession::mode() const {
    return m_mode;
}

void MaximaSession::setMode(MaximaSession::Mode mode)
{
    m_mode = mode;
}

Cantor::Expression* MaximaSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave, bool internal)
{
    qDebug() << "evaluating: " << cmd;
    auto* expr = new MaximaExpression(this, internal);
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
   QString out = QString::fromLocal8Bit(m_process->readAllStandardError());

   if(expressionQueue().size()>0)
   {
       auto* expr = expressionQueue().first();
       expr->parseError(out);
   }
}

void MaximaSession::readStdOut()
{
    QString out = QString::fromLocal8Bit(m_process->readAllStandardOutput());
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

    auto* expr = expressionQueue().first();
    if (!expr)
        return; //should never happen

    qDebug()<<"output: " << m_cache;
    expr->parseOutput(m_cache);
    m_cache.clear();
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

void MaximaSession::runFirstExpression()
{
    qDebug()<<"running next expression";
    if (!m_process)
        return;

    if(!expressionQueue().isEmpty())
    {
        auto* expr = expressionQueue().first();
        const auto& command = expr->internalCommand();
        connect(expr, &Cantor::Expression::statusChanged, this, &Session::currentExpressionStatusChanged);

        if(command.isEmpty())
        {
            qDebug()<<"empty command";
            static_cast<MaximaExpression*>(expr)->forceDone();
        }
        else
        {
            expr->setStatus(Cantor::Expression::Computing);
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
        if(m_process && m_process->state() != QProcess::NotRunning)
        {
#ifndef Q_OS_WIN
            const int pid = m_process->processId();
            kill(pid, SIGINT);
#else
            ; //TODO: interrupt the process on windows
#endif
        }
       for (auto* expression : expressionQueue())
            expression->setStatus(Cantor::Expression::Interrupted);

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
    qDebug()<<"restarting maxima cooldown: " << m_justRestarted;

    if(!m_justRestarted)
    {
        emit error(i18n("Maxima crashed. restarting..."));
        //remove the command that caused maxima to crash (to avoid infinite loops)
        if(!expressionQueue().isEmpty())
            expressionQueue().removeFirst();

        m_justRestarted=true;
        QTimer::singleShot(1000, this, SLOT(restartsCooledDown()));

        disconnect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(restartMaxima()));
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
    if (m_process)
    {
        //we use the lisp command to set the variable, as those commands
        //don't mess with the labels and history
        const QString& val = QLatin1String((enable ? "t":"nil"));
        evaluateExpression(QString::fromLatin1(":lisp(setf $display2d %1)").arg(val), Cantor::Expression::DeleteOnFinish, true);
    }

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

void MaximaSession::write(const QString& exp) {
    qDebug()<<"################################## EXPRESSION START ###############################################";
    qDebug()<<"sending expression to maxima process: " << exp;
    m_process->write(exp.toUtf8());
}
