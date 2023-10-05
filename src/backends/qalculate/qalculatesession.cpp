/*
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2011 Matteo Agostinelli <agostinelli@gmail.com>
    SPDX-FileCopyrightText: 2022 Alexander Semke (alexander.semke@web.de)

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "settings.h"

#include "qalculatesession.h"
#include "qalculatecompletionobject.h"
#include "qalculatehighlighter.h"
#include "defaultvariablemodel.h"

#include <QProcess>
#include <QRegularExpression>

#include <libqalculate/Calculator.h>
#include <libqalculate/ExpressionItem.h>
#include <libqalculate/Unit.h>
#include <libqalculate/Prefix.h>
#include <libqalculate/Function.h>

#include "qalculatesyntaxhelpobject.h"

QalculateSession::QalculateSession( Cantor::Backend* backend)
    : Session(backend),
      m_variableModel(new Cantor::DefaultVariableModel(this)),
      m_process(nullptr),
      m_currentExpression(nullptr),
      m_isSaveCommand(false)
{
    /*
        qalc does all of this by default but we still need the CALCULATOR instance for plotting
        graphs
    */

    if ( !CALCULATOR ) {
             new Calculator();
             CALCULATOR->loadGlobalDefinitions();
             CALCULATOR->loadLocalDefinitions();
             CALCULATOR->loadExchangeRates();
    }
}

QalculateSession::~QalculateSession()
{
    CALCULATOR->abort();
    if(m_process)
    {
        m_process->kill();
        m_process->deleteLater();
        m_process = nullptr;
    }
}

void QalculateSession::login()
{
    if (m_process)
        return;

    Q_EMIT loginStarted();
    qDebug() << "login started";

    /* we will , most probably, use autoscripts for setting the mode , evaluate options, print options etc */

    // if(!QalculateSettings::autorunScripts().isEmpty()){
    //     QString autorunScripts = QalculateSettings::self()->autorunScripts().join(QLatin1String("\n"));
    //
    //     evaluateExpression(autorunScripts, QalculateExpression::DeleteOnFinish);
    // }

    /*
        set up the process here. The program path , arguments(if any),channel modes , and connections should all be set up here.
        once the setup is complete, start the process and inform the worksheet that we are ready
    */
    m_process = new QProcess(this);

    m_process->setProgram(QStandardPaths::findExecutable(QLatin1String("qalc")));
    QStringList args{QLatin1String("-s"), QLatin1String("color 0")}; // switch off the colored output
    m_process->setArguments(args);
    m_process->setProcessChannelMode(QProcess::SeparateChannels);

    connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(readOutput()));
    connect(m_process, SIGNAL(readyReadStandardError()), this, SLOT(readError()));
    connect(m_process, SIGNAL(started()), this, SLOT(processStarted()));

    m_process->start();

    changeStatus(Session::Done);
    Q_EMIT loginDone();
}

void QalculateSession::readOutput()
{
        while(m_process->bytesAvailable()) {
                m_output.append(QString::fromLocal8Bit(m_process->readLine()));
                qDebug() << m_output;
        }

        if(m_currentExpression && !m_output.isEmpty() && m_output.trimmed().endsWith(QLatin1String(">"))) {

                // check if the commandQueue is empty or not . if it's not empty run the "runCommandQueue" function.
                // store the output in finalOutput and clear m_output

                if(m_currentCommand.trimmed().isEmpty())
                    m_output.clear();

                if(!m_output.toLower().contains(QLatin1String("error")) && m_isSaveCommand) {
                        storeVariables(m_currentCommand, m_output);
                        m_isSaveCommand = false;
                }

                m_output = m_output.trimmed();
                m_output.remove(m_currentCommand);
                if (!m_output.isEmpty())
                    m_finalOutput.append(m_output);

                // we tried to perform a save operation but failed(see parseSaveCommand()).In such a case
                // m_output will be empty but m_saveError will contain the error message.
                if(!m_saveError.isEmpty()) {
                    m_finalOutput.append(m_saveError);
                    m_saveError.clear();
                }

                m_finalOutput.append(QLatin1String("\n"));
                m_output.clear();



                if (!m_commandQueue.isEmpty())
                    runCommandQueue();
                else {
                    qDebug () << "parsing output: " << m_finalOutput;
                    m_currentExpression->parseOutput(m_finalOutput);
                    m_finalOutput.clear();
                }
        }
}

void QalculateSession::storeVariables(QString& currentCmd, QString output)
{

    // internally we pass save(value,variable) command to qlac to save the variables. see parseSaveCommand()
    // TODO: if the user if trying to override a default variable(constants etc) or an existing variable, ask the user if he/she wants to override it or not.

    qDebug() << "save command " << currentCmd;

    /**
        if we have reached here, we expect our variable model to be updated with new variables.
        In case the variable model is not updated, it most probably because we were not able to successfully parse the
        current command and output to extract variable and value

        This is probably not the best way to get the variable and value.
        But since qalc does not  provide a way to get the list of variables, we will have to stick to parsing
    **/
    QRegularExpression regex;
    // find the value
    regex.setPattern(QStringLiteral("^[\\s\\w\\W]+=\\s*([\\w\\W]+)$"));
    QRegularExpressionMatch match = regex.match(output);
    QString value;
    if(match.hasMatch()) {
        value = match.captured(1).trimmed();
        value.replace(QLatin1String("\n"), QLatin1String(""));
        value.remove(QLatin1String(">"));
    }

    //find the varaiable.
    // ex1: currentCmd = save(10, var_1,category, title): var_1 = variable
    // ex2: currentCmd = save(planet(jupter,mass), jupiter_mass, category, title): jupiter_mass = variable

    //  regex.setPattern(QLatin1String("\\s*save\\s*\\(\\s*[\\s\\w]+\\s*,([\\s\\w]+),*[\\w\\W]*\\)\\s*;*$|\\s*save\\s*\\(\\s*[\\s\\w\\W]+\\)\\s*,([\\s\\w]+),*[\\w\\W]*\\)\\s*;*$"));

    regex.setPattern(QStringLiteral("^\\s*save\\s*\\("
                                    "(?:.+?(?:\\(.+?,.+?\\))|(?:[^,()]+?)),"
                                    "(.+?),"
                                    "(?:.+?),"
                                    "(?:.+?)\\)\\s*;?$"));
    QString var;
    match = regex.match(currentCmd);
    if (match.hasMatch()) {
        var = match.captured(1).trimmed();
        var.replace(QLatin1String("\n"), QLatin1String(""));
        var.remove(QLatin1String(">"));
    }
    if(!value.isEmpty() && !var.isEmpty())
        variables.insert(var, value);
}

void QalculateSession::readError()
{
    QString error =  QLatin1String(m_process->readAllStandardError());
    if(m_currentExpression) {
        m_currentExpression->parseError(error);
    }
}

void QalculateSession::processStarted()
{
    qDebug() << "process  started " << m_process->program() << m_process->processId();
}

void QalculateSession::logout()
{
    qDebug () << "logging out";
    if (!m_process)
        return;

    if(status() == Cantor::Session::Running)
        interrupt();

    m_process->write("quit\n");
    if(!m_process->waitForFinished(1000))
        m_process->kill();
    m_process->deleteLater();
    m_process = nullptr;

    Session::logout();
}

void QalculateSession::interrupt()
{
    qDebug () << "interrupting .... ";
    if(m_currentExpression)
        m_currentExpression->interrupt();

    m_commandQueue.clear();
    m_expressionQueue.clear();
    m_output.clear();
    m_finalOutput.clear();
    m_currentCommand.clear();
    m_currentExpression = nullptr;

}

void QalculateSession::runExpression()
{
    const auto commands = m_currentExpression->command().split(QLatin1Char('\n'));
    for(const QString& cmd : commands) {
        m_commandQueue.enqueue(cmd);
    }
    runCommandQueue();
}


void QalculateSession::runCommandQueue()
{
    if (!m_commandQueue.isEmpty()) {
        m_currentCommand = m_commandQueue.dequeue();
        // parse the current command if it's a save/load/store command
        if( m_currentCommand.toLower().trimmed().startsWith(QLatin1String("save")) ||
            m_currentCommand.toLower().trimmed().startsWith(QLatin1String("store")) ||
            m_currentCommand.trimmed().startsWith(QLatin1String("saveVariables"))) {

                m_currentCommand = parseSaveCommand(m_currentCommand);
            }

        m_currentCommand = m_currentCommand.trimmed();
        m_currentCommand += QLatin1String("\n");
        m_process->write(m_currentCommand.toLocal8Bit());
    }
}

QString QalculateSession::parseSaveCommand(QString& currentCmd)
{
    /*
        make sure the command is:
        * formatted correctly. e.g if the command is save(value,variable), we have to make sure that there is no space between save and '(', otherwise qalc
          waits for user input which is not supported by us as of now
        * supported save commands: save(value,variable,[category],[title]), save definitions, save mode, save var, store var,
        saveVariables filename
    */

    QRegularExpression regex;
    regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);

    regex.setPattern(QStringLiteral("^\\s*save\\s*definitions\\s*$"));
    if(regex.match(currentCmd).hasMatch()) {
        // save the variables in  ~/.cantor/backends/qalculate/definitions
        currentCmd.clear();
        return currentCmd;
    }

    regex.setPattern(QStringLiteral("^\\s*save\\s*mode\\s*$"));
    if(regex.match(currentCmd).hasMatch()) {
        // save the mode in ~/.cantor/backends/qalculate/cantor_qalc.cfg
        currentCmd.clear();
        return currentCmd;
    }

    regex.setPattern(QStringLiteral("^\\s*saveVariables\\s*[\\w\\W]+$"));
    if(regex.match(currentCmd).hasMatch()) {
        // save the variables in a file
        currentCmd.clear();
        return currentCmd;
    }

    regex.setPattern(QStringLiteral("^\\s*store\\s*([a-zA-Z_]+[\\w]*)|\\s*save\\s*([a-zA-Z_]+[\\w]*)$"));
    QRegularExpressionMatch match = regex.match(currentCmd);
    if(match.hasMatch()) {
        m_isSaveCommand = true;

        QString str = match.captured(1).trimmed();
        if (str.isEmpty())
            str = match.captured(2).trimmed();

        currentCmd = QStringLiteral("save(%1, %2)").arg(QStringLiteral("ans"), str);

        return currentCmd;
    }

    regex.setPattern(QStringLiteral("^\\s*save\\s*(\\([\\w\\W]+\\))\\s*;*$"));
    match = regex.match(currentCmd);
    if(match.hasMatch()) {
        m_isSaveCommand = true;
        currentCmd = QStringLiteral("save%1").arg(match.captured(1).trimmed());
        return currentCmd;
    }

    /*
        If we have not returned by this point, it's because:
        * we did not parse the save command properly. This might be due to malformed regular expressions.
        * or the commnad given by the user is malformed. More likely to happen
        In both these cases we will simply return an empty string because we don't want qalc to run malformed queries,
        else it would wait for user input and hence Qprocess would never return a complete output and the expression will remain in
        'calculating' state
    */
    m_saveError =  currentCmd + QLatin1String("\nError: Could not save.\n");
    return QLatin1String("");
}

//TODO: unify with the funcion in the base class
void QalculateSession::currentExpressionStatusChanged(Cantor::Expression::Status status)
{
    // depending on the status of the expression change the status of the session;
    switch (status) {
        case Cantor::Expression::Computing:
            break;
        case Cantor::Expression::Interrupted:
            changeStatus(Cantor::Session::Done);
            break;
        case Cantor::Expression::Queued:
            break;
        case Cantor::Expression::Done:
        case Cantor::Expression::Error:
            qDebug() << " ******  STATUS   " << status;
            changeStatus(Cantor::Session::Done);
            if(m_expressionQueue.size() > 0)
                m_expressionQueue.dequeue();
            if(!m_expressionQueue.isEmpty())
                runExpressionQueue();
    }
}

Cantor::Expression* QalculateSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave, bool internal)
{
    qDebug() << " ** evaluating expression: " << cmd;
    qDebug() << " size of expression queue: " << m_expressionQueue.size();

    changeStatus(Cantor::Session::Running);

    QalculateExpression* expr = new QalculateExpression(this, internal);
    expr->setFinishingBehavior(behave);
    expr->setCommand(cmd);

    m_expressionQueue.enqueue(expr);
    runExpressionQueue();

    return expr;
}

void QalculateSession::runExpressionQueue()
{
    if(!m_expressionQueue.isEmpty()) {

        if(!m_currentExpression)
            m_currentExpression = m_expressionQueue.head();

        else {
            /* there was some expression that was being executed by cantor. We run the new expression only
               if the current expression's status is 'Done' or 'Error', if not , we simply return
            */
            Cantor::Expression::Status expr_status = m_currentExpression->status();
            if(expr_status != Cantor::Expression::Done &&  expr_status != Cantor::Expression::Error)
                return;
        }

        m_currentExpression = m_expressionQueue.head();
        connect(m_currentExpression, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SLOT(currentExpressionStatusChanged(Cantor::Expression::Status)));
        // start processing the expression
        m_currentExpression->evaluate();
    }
}


Cantor::CompletionObject* QalculateSession::completionFor(const QString& command, int index)
{
    return new QalculateCompletionObject(command, index, this);
}

Cantor::SyntaxHelpObject* QalculateSession::syntaxHelpFor(const QString& cmd)
{
    return new QalculateSyntaxHelpObject(cmd, this);
}

QSyntaxHighlighter* QalculateSession::syntaxHighlighter(QObject* parent)
{
    return new QalculateHighlighter(parent);
}

Cantor::DefaultVariableModel* QalculateSession::variableModel() const
{
    return m_variableModel;
}
