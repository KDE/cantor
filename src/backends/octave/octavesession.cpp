/*
    SPDX-FileCopyrightText: 2010 Miha Čančula <miha.cancula@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <random>
#include "octavesession.h"
#include "octaveexpression.h"
#include "octavecompletionobject.h"
#include "octavesyntaxhelpobject.h"
#include "octavehighlighter.h"
#include "result.h"
#include "textresult.h"
#include <backend.h>

#include "settings.h"

#include <KProcess>
#include <KDirWatch>
#include <KLocalizedString>
#include <KMessageBox>

#include <QTimer>
#include <QFile>
#include <QDir>
#include <QStringRef>

#ifndef Q_OS_WIN
#include <signal.h>
#endif

#include "octavevariablemodel.h"

const QRegularExpression OctaveSession::PROMPT_UNCHANGEABLE_COMMAND = QRegularExpression(QStringLiteral("^(?:,|;)+$"));

OctaveSession::OctaveSession(Cantor::Backend* backend) : Session(backend),
m_prompt(QStringLiteral("CANTOR_OCTAVE_BACKEND_PROMPT:([0-9]+)> ")),
m_subprompt(QStringLiteral("CANTOR_OCTAVE_BACKEND_SUBPROMPT:([0-9]+)> "))
{
    setVariableModel(new OctaveVariableModel(this));
}

OctaveSession::~OctaveSession()
{
    if (m_process)
    {
        m_process->kill();
        m_process->deleteLater();
        m_process = nullptr;
    }
}

void OctaveSession::login()
{
    qDebug() << "login";
    if (m_process)
        return;

    emit loginStarted();

    m_process = new KProcess(this);
    QStringList args;
    args << QLatin1String("--silent");
    args << QLatin1String("--interactive");
    args << QLatin1String("--persist");

    // Setting prompt and subprompt
    args << QLatin1String("--eval");
    args << QLatin1String("PS1('CANTOR_OCTAVE_BACKEND_PROMPT:\\#> ');");
    args << QLatin1String("--eval");
    args << QLatin1String("PS2('CANTOR_OCTAVE_BACKEND_SUBPROMPT:\\#> ');");

    // Add the cantor script directory to octave script search path
    const QStringList& scriptDirs = locateAllCantorFiles(QLatin1String("octavebackend"), QStandardPaths::LocateDirectory);
    if (scriptDirs.isEmpty())
        qCritical() << "Octave script directory not found, needed for integrated plots";
    else
    {
        for (const QString& dir : scriptDirs)
            args << QLatin1String("--eval") << QString::fromLatin1("addpath \"%1\";").arg(dir);
    }

    // Do not show extra text in help commands
    args << QLatin1String("--eval");
    args << QLatin1String("suppress_verbose_help_message(1);");

    m_process->setProgram ( OctaveSettings::path().toLocalFile(), args );
    qDebug() << "starting " << m_process->program();
    m_process->setOutputChannelMode ( KProcess::SeparateChannels );
    m_process->start();
    m_process->waitForStarted();

    connect ( m_process, SIGNAL (readyReadStandardOutput()), SLOT (readOutput()) );
    connect ( m_process, SIGNAL (readyReadStandardError()), SLOT (readError()) );
    connect ( m_process, SIGNAL (error(QProcess::ProcessError)), SLOT (processError()) );

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> rand_dist(0, 999999999);
    m_plotFilePrefixPath =
        QDir::tempPath()
        + QLatin1String("/cantor_octave_")
        + QString::number(m_process->processId())
        + QLatin1String("_")
        + QString::number(rand_dist(mt))
        + QLatin1String("_");

    if(!OctaveSettings::self()->autorunScripts().isEmpty()){
        QString autorunScripts = OctaveSettings::self()->autorunScripts().join(QLatin1String("\n"));

        evaluateExpression(autorunScripts, OctaveExpression::DeleteOnFinish, true);
        updateVariables();
    }

    if (!m_worksheetPath.isEmpty())
    {
        static const QString mfilenameTemplate = QLatin1String(
            "function retval = mfilename(arg_mem = \"\")\n"
                "type_info=typeinfo(arg_mem);\n"
                "if (strcmp(type_info, \"string\"))\n"
                    "if (strcmp(arg_mem, \"fullpath\"))\n"
                        "retval = \"%1\";\n"
                    "elseif (strcmp(arg_mem, \"fullpathext\"))\n"
                        "retval = \"%2\";\n"
                    "else\n"
                        "retval = \"script\";\n"
                    "endif\n"
                "else\n"
                    "error(\"wrong type argument '%s'\", type_info)\n"
                "endif\n"
            "endfunction"
        );
        const QString& worksheetDirPath = QFileInfo(m_worksheetPath).absoluteDir().absolutePath();
        const QString& worksheetPathWithoutExtension = m_worksheetPath.mid(0, m_worksheetPath.lastIndexOf(QLatin1Char('.')));

        evaluateExpression(QLatin1String("cd ")+worksheetDirPath, OctaveExpression::DeleteOnFinish, true);
        evaluateExpression(mfilenameTemplate.arg(worksheetPathWithoutExtension, m_worksheetPath), OctaveExpression::DeleteOnFinish, true);
    }

    changeStatus(Cantor::Session::Done);
    emit loginDone();
    qDebug()<<"login done";
}

void OctaveSession::setWorksheetPath(const QString& path)
{
    m_worksheetPath = path;
}

void OctaveSession::logout()
{
    qDebug()<<"logout";

    if(!m_process)
        return;

    disconnect(m_process, nullptr, this, nullptr);

    if(status() == Cantor::Session::Running)
        interrupt();

    m_process->write("exit\n");
    qDebug()<<"send exit command to octave";

    if(!m_process->waitForFinished(1000))
    {
        m_process->kill();
        qDebug()<<"octave still running, process kill enforced";
    }
    m_process->deleteLater();
    m_process = nullptr;

    if (!m_plotFilePrefixPath.isEmpty())
    {
        int i = 0;
        const QString& extension = OctaveExpression::plotExtensions[OctaveSettings::inlinePlotFormat()];
        QString filename = m_plotFilePrefixPath + QString::number(i) + QLatin1String(".") + extension;
        while (QFile::exists(filename))
        {
            QFile::remove(filename);
            i++;
            filename = m_plotFilePrefixPath + QString::number(i) + QLatin1String(".") + extension;
        }
    }

    expressionQueue().clear();

    m_output.clear();
    m_previousPromptNumber = 1;
    m_isIntegratedPlotsEnabled = false;
    m_isIntegratedPlotsSettingsEnabled = false;

    Session::logout();

    qDebug()<<"logout done";
}

void OctaveSession::interrupt()
{
    qDebug() << expressionQueue().size();
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

        // Cleanup inner state and call octave prompt printing
        // If we move this code for interruption to Session, we need add function for
        // cleaning before setting Done status
        m_output.clear();
        m_process->write("\n");

        qDebug()<<"done interrupting";
    }

    changeStatus(Cantor::Session::Done);
}

void OctaveSession::processError()
{
    qDebug() << "processError";
    emit error(m_process->errorString());
}

Cantor::Expression* OctaveSession::evaluateExpression(const QString& command, Cantor::Expression::FinishingBehavior behavior, bool internal )
{
    if (!internal)
        updateGraphicPackagesFromSettings();

    qDebug() << "evaluating: " << command;
    auto* expression = new OctaveExpression(this, internal);
    expression->setCommand ( command );
    expression->setFinishingBehavior(behavior);
    expression->evaluate();

    return expression;
}

void OctaveSession::runFirstExpression()
{
    auto* expression = expressionQueue().first();
    connect(expression, &Cantor::Expression::statusChanged, this, &OctaveSession::currentExpressionStatusChanged);

    const auto& command = expression->internalCommand();
    expression->setStatus(Cantor::Expression::Computing);
    if (isDoNothingCommand(command))
        expression->setStatus(Cantor::Expression::Done);
    else
        m_process->write(command.toLocal8Bit());
}

void OctaveSession::readError()
{
    qDebug() << "readError";
    QString error = QString::fromLocal8Bit(m_process->readAllStandardError());
    if (!expressionQueue().isEmpty() && !error.isEmpty())
    {
        auto* const exp = expressionQueue().first();
        if (m_syntaxError)
        {
            m_syntaxError = false;
            exp->parseError(i18n("Syntax Error"));
        }
        else
            exp->parseError(error);

        m_output.clear();
    }
}

void OctaveSession::readOutput()
{
    qDebug() << "readOutput";
    while (m_process->bytesAvailable() > 0)
    {
        QString line = QString::fromLocal8Bit(m_process->readLine());
        qDebug()<<"start parsing " << "  " << line;
        QRegularExpressionMatch match = m_prompt.match(line);
        if (match.hasMatch())
        {
            const int promptNumber = match.captured(1).toInt();
            // Add all text before prompt, if exists
            m_output += QStringRef(&line, 0, match.capturedStart(0)).toString();
            if (!expressionQueue().isEmpty())
            {
                const QString& command = expressionQueue().first()->command();
                if (m_previousPromptNumber + 1 == promptNumber || isSpecialOctaveCommand(command))
                {
                    if (!expressionQueue().isEmpty())
                    {
                        readError();
                        expressionQueue().first()->parseOutput(m_output);
                    }
                }
                else
                {
                    // Error command don't increase octave prompt number (usually, but not always)
                    readError();
                }
            }
            m_previousPromptNumber = promptNumber;
            m_output.clear();
        }
        else if ((match = m_subprompt.match(line)).hasMatch()
                 && match.captured(1).toInt() == m_previousPromptNumber)
        {
            // User don't write finished octave statement (for example, write 'a = [1,2, ' only), so
            // octave print subprompt and waits input finish.
            m_syntaxError = true;
            qDebug() << "subprompt catch";
            m_process->write(")]'\"\n"); // force exit from subprompt
            m_output.clear();
        }
        else
            m_output += line;
    }
}

void OctaveSession::currentExpressionStatusChanged(Cantor::Expression::Status status)
{
    qDebug() << "currentExpressionStatusChanged" << status << expressionQueue().first()->command();
    switch (status)
    {
    case Cantor::Expression::Done:
    case Cantor::Expression::Error:
        finishFirstExpression();
        break;

    default:
        break;
    }
}

Cantor::CompletionObject* OctaveSession::completionFor(const QString& cmd, int index)
{
    return new OctaveCompletionObject(cmd, index, this);
}

Cantor::SyntaxHelpObject* OctaveSession::syntaxHelpFor(const QString& cmd)
{
    return new OctaveSyntaxHelpObject(cmd, this);
}

QSyntaxHighlighter* OctaveSession::syntaxHighlighter(QObject* parent)
{
    return new OctaveHighlighter(parent, this);
}

void OctaveSession::runSpecificCommands()
{
    m_process->write("figure(1,'visible','off')");
}

bool OctaveSession::isDoNothingCommand(const QString& command)
{
    return PROMPT_UNCHANGEABLE_COMMAND.match(command).hasMatch()
           || command.isEmpty() || command == QLatin1String("\n");
}

bool OctaveSession::isSpecialOctaveCommand(const QString& command)
{
    return command.contains(QLatin1String("completion_matches"));
}

bool OctaveSession::isIntegratedPlotsEnabled() const
{
    return m_isIntegratedPlotsEnabled;
}

QString OctaveSession::plotFilePrefixPath() const
{
    return m_plotFilePrefixPath;
}

void OctaveSession::updateGraphicPackagesFromSettings()
{
    if (m_isIntegratedPlotsSettingsEnabled == OctaveSettings::integratePlots())
        return;

    if (m_isIntegratedPlotsEnabled && OctaveSettings::integratePlots() == false)
    {
        updateEnabledGraphicPackages(QList<Cantor::GraphicPackage>());
        m_isIntegratedPlotsEnabled = false;
        m_isIntegratedPlotsSettingsEnabled = OctaveSettings::integratePlots();
        return;
    }
    else if (!m_isIntegratedPlotsEnabled && OctaveSettings::integratePlots() == true)
    {
        bool isIntegratedPlots = OctaveSettings::integratePlots();
        if (isIntegratedPlots)
        {
            QString filename = QDir::tempPath() + QLatin1String("/cantor_octave_plot_integration_test.txt");
            QFile::remove(filename); // Remove previous file, if precents
            int test_number = rand() % 1000;

            QStringList args;
            args << QLatin1String("--no-init-file");
            args << QLatin1String("--no-gui");
            args << QLatin1String("--eval");
            args << QString::fromLatin1("file_id = fopen('%1', 'w'); fdisp(file_id, %2); fclose(file_id);").arg(filename).arg(test_number);

            QString errorMsg;
            isIntegratedPlots = Cantor::Backend::testProgramWritable(
                OctaveSettings::path().toLocalFile(),
                args,
                filename,
                QString::number(test_number),
                &errorMsg
            );

            // If we in this branch, then isIntegratedPlots was true, but if it false now, then it means, that the writable test is failed
            if (isIntegratedPlots == false)
            {
                KMessageBox::error(nullptr,
                    i18n("Plot integration test failed.")+
                    QLatin1String("\n\n")+
                    errorMsg+
                    QLatin1String("\n\n")+
                    i18n("The integration of plots will be disabled."),
                    i18n("Cantor")
                );
            }
        }

        m_isIntegratedPlotsEnabled = isIntegratedPlots;
        m_isIntegratedPlotsSettingsEnabled = OctaveSettings::integratePlots();

        if (m_isIntegratedPlotsEnabled)
            updateEnabledGraphicPackages(backend()->availableGraphicPackages());
        else
            updateEnabledGraphicPackages(QList<Cantor::GraphicPackage>());
    }
}

QString OctaveSession::graphicPackageErrorMessage(QString packageId) const
{
    QString text;

    if (packageId == QLatin1String("gr")) {
        return i18n(
            "The plot integration doesn't work because Cantor found, that Octave can't create plots, "
            "because there are no graphical backends for it: this conclusion was made on the basis of empty "
            "output from available_graphics_toolkits() function. Looks like you should install some "
            "additional OS packages, like gnuplot, fltk or qt for possibility to create plots."
        );
    }
    return text;
}
