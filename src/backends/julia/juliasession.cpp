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
    Copyright (C) 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
 */
#include "juliasession.h"

#include <KProcess>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QStandardPaths>

#include "defaultvariablemodel.h"

#include "juliaexpression.h"
#include "settings.h"
#include "juliahighlighter.h"
#include "juliakeywords.h"
#include "juliaextensions.h"
#include "juliabackend.h"
#include "juliacompletionobject.h"

JuliaSession::JuliaSession(Cantor::Backend *backend)
    : Session(backend)
    , m_process(nullptr)
    , m_interface(nullptr)
    , m_currentExpression(nullptr)
    , m_variableModel(new Cantor::DefaultVariableModel(this))
{
}

void JuliaSession::login()
{
    if (m_process) {
        m_process->deleteLater();
    }

    m_process = new KProcess(this);
    m_process->setOutputChannelMode(KProcess::SeparateChannels);

    (*m_process)
        << QStandardPaths::findExecutable(QLatin1String("cantor_juliaserver"));

    m_process->start();

    m_process->waitForStarted();
    m_process->waitForReadyRead();
    QTextStream stream(m_process->readAllStandardOutput());

    QString readyStatus = QLatin1String("ready");
    while (m_process->state() == QProcess::Running) {
        const QString &rl = stream.readLine();
        if (rl == readyStatus) {
            break;
        }
    }

    if (!QDBusConnection::sessionBus().isConnected()) {
        qWarning() << "Can't connect to the D-Bus session bus.\n"
                      "To start it, run: eval `dbus-launch --auto-syntax`";
        return;
    }

    const QString &serviceName =
        QString::fromLatin1("org.kde.Cantor.Julia-%1").arg(m_process->pid());

    m_interface = new QDBusInterface(
        serviceName,
        QString::fromLatin1("/"),
        QString(),
        QDBusConnection::sessionBus()
    );

    if (!m_interface->isValid()) {
        qWarning() << QDBusConnection::sessionBus().lastError().message();
        return;
    }

    m_interface->call(
        QString::fromLatin1("login"),
        JuliaSettings::self()->replPath().path()
    );

    listVariables();

    // Plots integration
    if (integratePlots()) {
        runJuliaCommand(
            QLatin1String("import GR; ENV[\"GKS_WSTYPE\"] = \"nul\"")
        );
    }

    emit ready();
}

void JuliaSession::logout()
{
    m_process->terminate();

    JuliaKeywords::instance()->clearVariables();
    JuliaKeywords::instance()->clearFunctions();
}

void JuliaSession::interrupt()
{
    if (m_process->pid()) {
        m_process->kill();
    }

    for (Cantor::Expression *e : m_runningExpressions) {
        e->interrupt();
    }

    m_runningExpressions.clear();
    changeStatus(Cantor::Session::Done);
}

Cantor::Expression *JuliaSession::evaluateExpression(
    const QString &cmd,
    Cantor::Expression::FinishingBehavior behave)
{
    JuliaExpression *expr = new JuliaExpression(this);

    changeStatus(Cantor::Session::Running);

    expr->setFinishingBehavior(behave);
    expr->setCommand(cmd);
    expr->evaluate();

    return expr;
}

Cantor::CompletionObject *JuliaSession::completionFor(
    const QString &command,
    int index)
{
    return new JuliaCompletionObject(command, index, this);
}

QSyntaxHighlighter *JuliaSession::syntaxHighlighter(QObject *parent)
{
    JuliaHighlighter *highlighter = new JuliaHighlighter(parent);
    QObject::connect(
        this, SIGNAL(updateHighlighter()), highlighter, SLOT(updateHighlight())
    );
    return highlighter;
}

void JuliaSession::runJuliaCommand(const QString &command) const
{
    m_interface->call(QLatin1String("runJuliaCommand"), command);
}

void JuliaSession::runJuliaCommandAsync(const QString &command)
{
    m_interface->callWithCallback(
        QLatin1String("runJuliaCommand"),
        {command},
        this,
        SLOT(onResultReady())
    );
}

void JuliaSession::onResultReady()
{
    m_currentExpression->finalize();
    m_runningExpressions.removeAll(m_currentExpression);

    listVariables();

    changeStatus(Cantor::Session::Done);
}

void JuliaSession::runExpression(JuliaExpression *expr)
{
    m_runningExpressions.append(expr);
    m_currentExpression = expr;
    runJuliaCommandAsync(expr->command());
}

QString JuliaSession::getStringFromServer(const QString &method)
{
    const QDBusReply<QString> &reply = m_interface->call(method);
    return (reply.isValid() ? reply.value() : reply.error().message());
}

QString JuliaSession::getOutput()
{
    return getStringFromServer(QLatin1String("getOutput"));
}

QString JuliaSession::getError()
{
    return getStringFromServer(QLatin1String("getError"));
}

bool JuliaSession::getWasException()
{
    const QDBusReply<bool> &reply =
        m_interface->call(QLatin1String("getWasException"));
    return reply.isValid() && reply.value();
}

void JuliaSession::listVariables()
{
    QStringList ignoredVariables;
    ignoredVariables // These are tech variables of juliaserver
        << QLatin1String("__originalSTDOUT__")
        << QLatin1String("__originalSTDERR__");

    // Wrapping removed marker to quotes
    auto rem_marker = QString::fromLatin1("\"%1\"")
        .arg(JuliaVariableManagementExtension::REMOVED_VARIABLE_MARKER);

    // Clear current symbols
    JuliaKeywords::instance()->clearVariables();
    JuliaKeywords::instance()->clearFunctions();

    QStringList processed_modules; // modules we have processed
    QStringList modules_to_process; // modules in queue
    modules_to_process << QLatin1String("__GLOBAL__"); // starting from global

    while (modules_to_process.size() > 0) {
        // Get from queue
        auto module = modules_to_process.front();
        modules_to_process.pop_front();
        if (processed_modules.contains(module)) {
            continue;
        }
        processed_modules << module;

        // Get whos(<module here>) output, maybe from cache
        QString whos_output;
        if (module == QLatin1String("__GLOBAL__")) {
            runJuliaCommand(QLatin1String("whos()"));
            whos_output = getOutput();
        } else {
            auto it = m_whos_cache.find(module);
            if (it == m_whos_cache.end()) {
                runJuliaCommand(QString::fromLatin1("whos(%1)").arg(module));
                whos_output = getOutput();
                m_whos_cache[module] = whos_output;
            } else {
                whos_output = it.value();
            }
        }

        // In this lists we will collect symbols to apply `show` to them
        // in one DBus call
        QStringList batchCommands;
        QStringList batchTypes;
        QStringList batchNames;
        for (auto line : whos_output.split(QLatin1String("\n"))) {
            QString name =
                line.simplified().split(QLatin1String(" ")).first().simplified();

            if (name.isEmpty()) { // some empty line
                continue;
            }

            QString type =
                line.simplified().split(QLatin1String(" ")).last().simplified();

            if (ignoredVariables.contains(name)) {
                // Ignored variable
                continue;
            }

            if (type == QLatin1String("Module")) {
                // Found module, place in queue
                modules_to_process.append(name);
                continue;
            }

            if (type == QLatin1String("Function")) {
                // Found function
                JuliaKeywords::instance()->addFunction(name);
                continue;
            }

            if (module != QLatin1String("__GLOBAL__")) {
                continue; // Don't add variables not included on global scope
            }

            // Add to batch
            batchCommands << QString::fromLatin1("show(%1);").arg(name);
            batchTypes << type;
            batchNames << name;
        }

        if (batchCommands.isEmpty()) {
            continue; // nothing to do
        }

        // Run batched command
        runJuliaCommand(
            batchCommands.join(QLatin1String("print(\"__CANTOR_DELIM__\");"))
        );
        auto values = getOutput().split(QLatin1String("__CANTOR_DELIM__"));

        for (int i = 0; i < values.size(); i++) {
            auto value = values[i].simplified();
            auto type = batchTypes[i];
            auto name = batchNames[i];

            if (type == QLatin1String("ASCIIString")) {
                if (value == rem_marker) {
                    // This is removed variable
                    m_variableModel->removeVariable(name);
                    continue;
                }
            }

            // Register variable
            m_variableModel->addVariable(name, value);
            JuliaKeywords::instance()->addVariable(name);
        }
    }

    emit updateHighlighter();
}

QAbstractItemModel *JuliaSession::variableModel()
{
    return m_variableModel;
}


bool JuliaSession::integratePlots()
{
    return JuliaSettings::integratePlots();
}


#include "juliasession.moc"
