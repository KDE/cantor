/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
*/

#include "sagesession.h"
#include "sageexpression.h"
#include "sagecompletionobject.h"
#include "sagehighlighter.h"
#include "settings.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>

#include <KLocalizedString>
#include <KMessageBox>

#ifndef Q_OS_WIN
#include <signal.h>
#endif

const QByteArray SageSession::SagePrompt = "sage: "; //Text, sage outputs after each command
const QByteArray SageSession::SageAlternativePrompt = "....: "; //Text, sage outputs when it expects further input

//some commands that are run after login
static QByteArray initCmd = "import os\n"\
                           "os.environ['PAGER'] = 'cat'                     \n "\
                           "sage.misc.pager.EMBEDDED_MODE = True           \n "\
                           "sage.misc.viewer.BROWSER=''                    \n "\
                           "sage.plot.plot3d.base.SHOW_DEFAULTS['viewer'] = 'tachyon' \n"\
                           "sage.misc.latex.EMBEDDED_MODE = True           \n "\
                           "%colors nocolor                                \n "\
                           "try: \n "\
                           "    SAGE_TMP = sage.misc.temporary_file.TMP_DIR_FILENAME_BASE.name \n "\
                           "except AttributeError: \n "\
                           "    SAGE_TMP = sage.misc.misc.SAGE_TMP \n "\
                           "print('%s %s' % ('____TMP_DIR____', SAGE_TMP))\n";

static QByteArray newInitCmd =
    "__CANTOR_IPYTHON_SHELL__=get_ipython()   \n "\
    "__CANTOR_IPYTHON_SHELL__.autoindent=False\n ";

static QByteArray endOfInitMarker = "print('____END_OF_INIT____')\n ";


SageSession::VersionInfo::VersionInfo(int major, int minor)
{
    m_major = major;
    m_minor = minor;
}

int SageSession::VersionInfo::majorVersion() const
{
    return m_major;
}

int SageSession::VersionInfo::minorVersion() const
{
    return m_minor;
}

bool SageSession::VersionInfo::operator==(VersionInfo other) const
{
    return m_major == other.m_major && m_minor==other.m_minor;
}

bool SageSession::VersionInfo::operator<(VersionInfo other) const
{
    return (m_major != -1 && other.m_major == -1) ||
        ( ((m_major !=- 1 && other.m_major != -1) || (m_major == other.m_major && m_major == -1) )
        && ( m_major < other.m_major || (m_major == other.m_major && m_minor < other.m_minor) ) );
}

bool SageSession::VersionInfo::operator<=(VersionInfo other) const
{
    return (*this < other)||(*this == other);
}

bool SageSession::VersionInfo::operator>(SageSession::VersionInfo other) const
{
    return !( (*this <= other ));
}

bool SageSession::VersionInfo::operator>=(SageSession::VersionInfo other) const
{
    return !( *this < other);
}

SageSession::SageSession(Cantor::Backend* backend) : Session(backend)
{
    connect(&m_dirWatch, &KDirWatch::created, this, &SageSession::fileCreated);
}

SageSession::~SageSession()
{
    if (m_process)
    {
        m_process->kill();
        m_process->deleteLater();
        m_process = nullptr;
    }
}

void SageSession::login()
{
    qDebug()<<"login";
    if (m_process)
        return;
    Q_EMIT loginStarted();

    updateSageVersion();

    QStringList arguments;
    arguments << QLatin1String("-q"); // suppress the banner
    arguments << QLatin1String("--simple-prompt"); // suppress the colorizing of the output

    m_process = new QProcess(this);
    m_process->start(SageSettings::self()->path().toLocalFile(), arguments);

    if (!m_process->waitForStarted())
    {
        changeStatus(Session::Disable);
        Q_EMIT error(i18n("Failed to start Sage, please check Sage installation."));
        Q_EMIT loginDone();
        delete m_process;
        m_process = nullptr;
        return;
    }

    connect(m_process, &QProcess::readyRead, this, &SageSession::readStdOut);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &SageSession::readStdOut);
    connect(m_process, &QProcess::readyReadStandardError, this, &SageSession::readStdErr);
    connect(m_process, &QProcess::errorOccurred, this, &SageSession::reportProcessError);
    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(processFinished(int,QProcess::ExitStatus)));

    // initialize the settings for embeded plots
    // TODO: right now the settings are evaluated during the login only and the user needs to re-login
    // or to restart the application to get the changes applied. A better logic would be to recognize
    // a plot command and to apply the changes "on the fly" as in maximasession for example or
    // to apply the changes when the user has modified the settings only. Both options are not
    // available for Sage yet and should be implemented later.
    if (SageSettings::self()->integratePlots())
    {
        // deactivate external viewers
        initCmd += "sage.misc.viewer.viewer.png_viewer('false')\n";
        initCmd += "sage.misc.viewer.viewer.pdf_viewer('false')\n";
    }
    else
    {
        initCmd += "sage.misc.viewer.viewer.png_viewer('true')\n";
        initCmd += "sage.misc.viewer.viewer.pdf_viewer('true')\n";
    }

    if (SageSettings::inlinePlotFormat() == 0) // PDF
        initCmd += "sage.repl.rich_output.get_display_manager().preferences.graphics = 'vector' \n";
    else // PNG
        initCmd += "sage.repl.rich_output.get_display_manager().preferences.graphics = 'raster' \n";

    // matplotlib's figure accepts the sizes in inches
    double w = SageSettings::plotWidth() / 2.54;
    double h = SageSettings::plotHeight() / 2.54;
    initCmd += "import matplotlib.pyplot as plt; plt.rcParams['figure.figsize'] = [" + QString::number(w).toLatin1() + ", " + QString::number(h).toLatin1() + "]\n";

    m_process->write(initCmd);

    // set the current working directory to the project directory
    // and save the path to the worksheet as variable "__file__"  -
    // this variable is usually set by the "os" package when running a script
    // but when it is run in an interpreter (like sage server) it is not set
    const auto& path = worksheetPath();
    if (!path.isEmpty())
    {
        auto cmd = QLatin1String("__file__ = '%1'").arg(path);
        evaluateExpression(cmd, Cantor::Expression::DeleteOnFinish, true);

        const auto& dir = QFileInfo(path).absoluteDir().absolutePath();
        cmd = QLatin1String("cd '%1'").arg(dir);
        evaluateExpression(cmd, Cantor::Expression::DeleteOnFinish, true);
    }

    //enable latex typesetting if needed
    const QString cmd = QLatin1String("__cantor_enable_typesetting(%1)");
    evaluateExpression(cmd.arg(isTypesettingEnabled() ? QLatin1String("true"):QLatin1String("false")),
                       Cantor::Expression::DeleteOnFinish);

    //auto-run scripts
    if(!SageSettings::self()->autorunScripts().isEmpty()){
        QString autorunScripts = SageSettings::self()->autorunScripts().join(QLatin1String("\n"));
        evaluateExpression(autorunScripts, SageExpression::DeleteOnFinish, true);
    }

    changeStatus(Session::Done);
    Q_EMIT loginDone();
}

void SageSession::logout()
{
    qDebug()<<"logout";

    if (!m_process)
        return;

    if(status() == Cantor::Session::Running)
        interrupt();

    disconnect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(processFinished(int,QProcess::ExitStatus)));

    m_process->write("exit\n");

    if(!m_process->waitForFinished(1000))
        m_process->kill();
    m_process->deleteLater();
    m_process = nullptr;

    m_isInitialized = false;
    m_waitingForPrompt = false;
    m_haveSentInitCmd = false;

    Session::logout();
}

Cantor::Expression* SageSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave, bool internal)
{
    qDebug() << "evaluating: " << cmd;
    auto* expr = new SageExpression(this, internal);
    expr->setFinishingBehavior(behave);
    expr->setCommand(cmd);
    expr->evaluate();

    return expr;
}

void SageSession::readStdOut()
{
    QString out = QString::fromUtf8(m_process->readAllStandardOutput());
    if (out.isEmpty())
        return;

    qDebug()<<"out: " << out;
    m_outputCache += out;

    if ( m_outputCache.contains( QLatin1String("___TMP_DIR___") ) )
    {
        int index = m_outputCache.indexOf(QLatin1String("___TMP_DIR___") )+14;
        int endIndex = m_outputCache.indexOf(QLatin1String("\n"), index);

        if(endIndex == -1)
            m_tmpPath = m_outputCache.mid( index ).trimmed();
        else
            m_tmpPath = m_outputCache.mid( index, endIndex-index ).trimmed();

        qDebug()<<"tmp path: "<<m_tmpPath;

        m_dirWatch.addDir(m_tmpPath, KDirWatch::WatchFiles);
    }

    if(!m_isInitialized)
    {
        // enforce the minimal version Sage 9.2 (released Oct 24, 2020).
        // this is the version supporting the --simple prompt CLI option that simplifies the initialization of sage,
        // s.a. the discussions in
        // https://github.com/sagemath/sage/issues/25363
        // https://bugs.kde.org/show_bug.cgi?id=408176
        if(updateSageVersion())
        {
            if(m_sageVersion <= SageSession::VersionInfo(9, 2))
            {
                const QString& message = i18n("Sage version %1.%2 is unsupported. Please update your installation to the versions 9.2 or higher.",
                                             m_sageVersion.majorVersion(), m_sageVersion.minorVersion());
                KMessageBox::error(nullptr, message, i18n("Unsupported Version"));
                interrupt();
                logout();
            }
            else
            {
                qDebug()<<"using the current set of commands";

                if(!m_haveSentInitCmd)
                {
                    m_process->write(newInitCmd);
                    defineCustomFunctions();
                    m_process->write(endOfInitMarker);
                    m_haveSentInitCmd=true;
                }
            }
        }
        else
        {
            const QString& message = i18n("Failed to determine the version of Sage. Please check your installation and the output of 'sage -v'.");
            KMessageBox::error(nullptr, message, i18n("Unsupported Version"));
            interrupt();
            logout();
        }
    }


    int indexOfEOI = m_outputCache.indexOf(QLatin1String("____END_OF_INIT____"));
    if(indexOfEOI != -1 && m_outputCache.indexOf(QLatin1String(SagePrompt), indexOfEOI) != -1)
    {
        qDebug() << "initialized";
        //out.remove("____END_OF_INIT____");
        //out.remove(SagePrompt);
        m_isInitialized = true;
        m_waitingForPrompt = false;
        runFirstExpression();
        m_outputCache.clear();
    }

    //If we are waiting for another prompt, drop every output
    //until a prompt is found
    if(m_isInitialized && m_waitingForPrompt)
    {
        qDebug() << "waiting for prompt";
        if(m_outputCache.contains(QLatin1String(SagePrompt)))
            m_waitingForPrompt = false;

        m_outputCache.clear();
        return;
    }

    if(m_isInitialized)
    {
        if (!expressionQueue().isEmpty())
        {
            auto* expr = expressionQueue().first();
            expr->parseOutput(m_outputCache);
        }
        m_outputCache.clear();
    }
}

void SageSession::readStdErr()
{
    qDebug()<<"reading stdErr";
    QString out = QLatin1String(m_process->readAllStandardError());
    if (!expressionQueue().isEmpty())
    {
        auto* expr = expressionQueue().first();
        expr->parseError(out);
    }
}

void SageSession::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    if(exitStatus == QProcess::CrashExit)
    {
        if(!expressionQueue().isEmpty())
        {
            static_cast<SageExpression*>(expressionQueue().last())
                ->onProcessError(i18n("The Sage process crashed while evaluating this expression"));
        }else
        {
            //We don't have an actual command. it crashed for some other reason, just show a plain error message box
            KMessageBox::error(nullptr, i18n("The Sage process crashed"), i18n("Cantor"));
        }
    }else
    {
        if(!expressionQueue().isEmpty())
        {
            static_cast<SageExpression*>(expressionQueue().last())
                ->onProcessError(i18n("The Sage process exited while evaluating this expression"));
        }else
        {
            //We don't have an actual command. it crashed for some other reason, just show a plain error message box
            KMessageBox::error(nullptr, i18n("The Sage process exited"), i18n("Cantor"));
        }
    }
}

void SageSession::reportProcessError(QProcess::ProcessError e)
{
    if(e == QProcess::FailedToStart)
    {
        changeStatus(Cantor::Session::Done);
        Q_EMIT error(i18n("Failed to start Sage"));
    }
}

void SageSession::runFirstExpression()
{
    if(!expressionQueue().isEmpty())
    {
        auto* expr = expressionQueue().first();

        if (m_isInitialized)
        {
            connect(expr, &Cantor::Expression::statusChanged, this, &Session::currentExpressionStatusChanged);

            QString command = expr->command();
            if(command.endsWith(QLatin1Char('?')) && !command.endsWith(QLatin1String("??")))
                command=QLatin1String("help(")+command.left(command.size()-1)+QLatin1Char(')');
            if(command.startsWith(QLatin1Char('?')))
                command=QLatin1String("help(")+command.mid(1)+QLatin1Char(')');
            command.append(QLatin1String("\n\n"));

            qDebug()<<"writing "<<command<<" to the process";
            expr->setStatus(Cantor::Expression::Computing);
            m_process->write(command.toUtf8());
        }
        else if (expressionQueue().size() == 1)
            // If queue contains one expression, it means, what we run this expression immediately (drop setting queued status)
            // TODO: Sage login is slow, so, maybe better mark this expression as queued for a login time
            expr->setStatus(Cantor::Expression::Queued);
    }
}

void SageSession::interrupt()
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
        m_outputCache.clear();

        qDebug()<<"done interrupting";
    }

    changeStatus(Cantor::Session::Done);
}

void SageSession::sendInputToProcess(const QString& input)
{
    m_process->write(input.toUtf8());
}

void SageSession::fileCreated( const QString& path )
{
    if (!SageSettings::self()->integratePlots())
        return;

    qDebug()<<"got a file " << path;
    if (!expressionQueue().isEmpty())
    {
        auto* expr = static_cast<SageExpression*>(expressionQueue().first());
        if (expr)
           expr->addFileResult( path );
    }
}

void SageSession::setTypesettingEnabled(bool enable)
{
    if (m_process)
    {
        //tell the sage server to enable/disable pretty_print
        const QString cmd = QLatin1String("__cantor_enable_typesetting(%1)");
        evaluateExpression(cmd.arg(enable ? QLatin1String("true"):QLatin1String("false")), Cantor::Expression::DeleteOnFinish);
    }

    Cantor::Session::setTypesettingEnabled(enable);
}

Cantor::CompletionObject* SageSession::completionFor(const QString& command, int index)
{
    return new SageCompletionObject(command, index, this);
}

QSyntaxHighlighter* SageSession::syntaxHighlighter(QObject* parent)
{
    return new SageHighlighter(parent);
}

SageSession::VersionInfo SageSession::sageVersion()
{
    return m_sageVersion;
}

void SageSession::defineCustomFunctions()
{
    //typesetting
    QString cmd = QLatin1String("def __cantor_enable_typesetting(enable):\n"\
                                "\t if(enable==true):\n "\
                                "\t \t %display typeset \n"\
                                "\t else: \n" \
                                "\t \t %display simple \n\n");

    sendInputToProcess(cmd);
}

bool SageSession::updateSageVersion()
{
    QProcess get_sage_version;
    get_sage_version.setProgram(SageSettings::self()->path().toLocalFile());
    get_sage_version.setArguments(QStringList() << QLatin1String("--version"));
    get_sage_version.start();
    if (!get_sage_version.waitForFinished(-1))
        return false;

    QString versionString = QString::fromLocal8Bit(get_sage_version.readLine());
    QRegularExpression versionExp(QLatin1String("(\\d+)\\.(\\d+)"));
    QRegularExpressionMatch version = versionExp.match(versionString);
    qDebug()<<"found version: " << version.capturedTexts();
    if(version.capturedTexts().length() == 3)
    {
        int major = version.capturedTexts().at(1).toInt();
        int minor = version.capturedTexts().at(2).toInt();
        m_sageVersion = SageSession::VersionInfo(major, minor);
        return true;
    }
    else
        return false;
}
