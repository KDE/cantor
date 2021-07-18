/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "sagesession.h"
#include "sageexpression.h"
#include "sagecompletionobject.h"
#include "sagehighlighter.h"

#include <QDebug>
#include <QRegularExpression>
#include <KPtyProcess>
#include <KPtyDevice>
#include <KLocalizedString>
#include <KMessageBox>
#include "settings.h"

#ifndef Q_OS_WIN
#include <signal.h>
#endif

const QByteArray SageSession::SagePrompt="sage: "; //Text, sage outputs after each command
const QByteArray SageSession::SageAlternativePrompt="....: "; //Text, sage outputs when it expects further input

//some commands that are run after login
static QByteArray initCmd="os.environ['PAGER'] = 'cat'                     \n "\
                           "sage.misc.pager.EMBEDDED_MODE = True           \n "\
                           "sage.misc.viewer.BROWSER=''                    \n "\
                           "sage.misc.viewer.viewer.png_viewer('false')         \n" \
                           "sage.plot.plot3d.base.SHOW_DEFAULTS['viewer'] = 'tachyon' \n"\
                           "sage.misc.latex.EMBEDDED_MODE = True           \n "\
                           "os.environ['PAGER'] = 'cat'                    \n "\
                           "%colors nocolor                                \n "\
                           "print('%s %s' % ('____TMP_DIR____', sage.misc.misc.SAGE_TMP))\n";

static QByteArray newInitCmd=
    "__CANTOR_IPYTHON_SHELL__=get_ipython()   \n "\
    "__CANTOR_IPYTHON_SHELL__.autoindent=False\n ";

static QByteArray legacyInitCmd=
    "__CANTOR_IPYTHON_SHELL__=__IPYTHON__   \n "  \
    "__CANTOR_IPYTHON_SHELL__.autoindent=False\n ";

static QByteArray endOfInitMarker="print('____END_OF_INIT____')\n ";



SageSession::VersionInfo::VersionInfo(int major, int minor)
{
    m_major=major;
    m_minor=minor;
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
    return m_major==other.m_major&&m_minor==other.m_minor;
}

bool SageSession::VersionInfo::operator<(VersionInfo other) const
{
    return (m_major!= -1 && other.m_major==-1) ||
        ( ((m_major!=-1 && other.m_major!=-1) || (m_major==other.m_major && m_major==-1) ) && ( m_major<other.m_major||(m_major==other.m_major&&m_minor<other.m_minor) ) );
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

SageSession::SageSession(Cantor::Backend* backend) : Session(backend),
m_process(nullptr),
m_isInitialized(false),
m_waitingForPrompt(false),
m_haveSentInitCmd(false)
{
    connect( &m_dirWatch, SIGNAL(created(QString)), this, SLOT(fileCreated(QString)) );
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
    emit loginStarted();

    m_process=new KPtyProcess(this);
    updateSageVersion();
    const QString& sageExecFile = SageSettings::self()->path().toLocalFile();
    if (false)
//  Reenable when https://trac.sagemath.org/ticket/25363 is merged
//  if (m_sageVersion >= SageSession::VersionInfo(8, 4))
        m_process->setProgram(sageExecFile, QStringList() << QLatin1String("--simple-prompt"));
    else
        {
        const QString& sageStartScript = locateCantorFile(QLatin1String("sagebackend/cantor-execsage"));
        m_process->setProgram(sageStartScript, QStringList(sageExecFile));
        }

    m_process->setOutputChannelMode(KProcess::SeparateChannels);
    m_process->setPtyChannels(KPtyProcess::AllChannels);
    m_process->pty()->setEcho(false);

    connect(m_process->pty(), SIGNAL(readyRead()), this, SLOT(readStdOut()));
    connect(m_process, SIGNAL(readyReadStandardError()), this, SLOT(readStdErr()));
    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(processFinished(int,QProcess::ExitStatus)));
    connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(reportProcessError(QProcess::ProcessError)));
    m_process->start();
    m_process->waitForStarted();

    m_process->pty()->write(initCmd);

    //save the path to the worksheet as variable "__file__"
    //this variable is usually set by the "os" package when running a script
    //but when it is run in an interpreter (like sage server) it is not set
    if (!m_worksheetPath.isEmpty())
    {
        const QString cmd = QLatin1String("__file__ = '%1'");
        evaluateExpression(cmd.arg(m_worksheetPath), Cantor::Expression::DeleteOnFinish, true);
    }

    if(!SageSettings::self()->autorunScripts().isEmpty()){
        QString autorunScripts = SageSettings::self()->autorunScripts().join(QLatin1String("\n"));
        evaluateExpression(autorunScripts, SageExpression::DeleteOnFinish, true);
    }

    changeStatus(Session::Done);
    emit loginDone();
}

void SageSession::logout()
{
    qDebug()<<"logout";

    if (!m_process)
        return;

    if(status() == Cantor::Session::Running)
        interrupt();

    disconnect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(processFinished(int,QProcess::ExitStatus)));

    m_process->pty()->write("exit\n");

    if(!m_process->waitForFinished(1000))
        m_process->kill();
    m_process->deleteLater();
    m_process = nullptr;

    //Run sage-cleaner to kill all the orphans
    KProcess::startDetached(SageSettings::self()->path().toLocalFile(),QStringList()<<QLatin1String("-cleaner"));

    m_isInitialized = false;
    m_waitingForPrompt = false;
    m_haveSentInitCmd = false;

    Session::logout();
}

Cantor::Expression* SageSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave, bool internal)
{
    qDebug()<<"evaluating: "<<cmd;
    SageExpression* expr=new SageExpression(this, internal);
    expr->setFinishingBehavior(behave);
    expr->setCommand(cmd);
    expr->evaluate();

    return expr;
}

void SageSession::readStdOut()
{
    m_outputCache.append(QString::fromUtf8(m_process->pty()->readAll()));
    qDebug()<<"out: "<<m_outputCache;

    if ( m_outputCache.contains( QLatin1String("___TMP_DIR___") ) )
    {
        int index=m_outputCache.indexOf(QLatin1String("___TMP_DIR___") )+14;
        int endIndex=m_outputCache.indexOf(QLatin1String("\n"), index);

        if(endIndex==-1)
            m_tmpPath=m_outputCache.mid( index ).trimmed();
        else
            m_tmpPath=m_outputCache.mid( index, endIndex-index ).trimmed();

        qDebug()<<"tmp path: "<<m_tmpPath;

        m_dirWatch.addDir( m_tmpPath, KDirWatch::WatchFiles );
    }

    if(!m_isInitialized)
    {
        if(updateSageVersion())
        {
            // After the update ipython5 somewhere around Sage 7.6, Cantor stopped working with Sage.
            // To fix this, we start Sage via a help wrapper that makes ipythong using simple prompt.
            // This method works starting with Sage 8.1"
            // Versions lower than 8.1 are not supported because of https://github.com/ipython/ipython/issues/9816
            if(m_sageVersion <= SageSession::VersionInfo(8, 0) && m_sageVersion >= SageSession::VersionInfo(7,4))
            {
                const QString message = i18n(
                    "Sage version %1.%2 is unsupported. Please update your installation "\
                    "to the supported versions to make it work with Cantor.", m_sageVersion.majorVersion(), m_sageVersion.minorVersion());
                KMessageBox::error(nullptr, message, i18n("Cantor"));
                interrupt();
                logout();
            }
            else if(m_sageVersion<=SageSession::VersionInfo(5, 7))
            {
                qDebug()<<"using an old version of sage: "<<m_sageVersion.majorVersion()<<"."<<m_sageVersion.minorVersion()<<". Using the old init command";
                if(!m_haveSentInitCmd)
                {
                    m_process->pty()->write(legacyInitCmd);
                    defineCustomFunctions();
                    m_process->pty()->write(endOfInitMarker);
                    m_haveSentInitCmd=true;
                }

            }
            else
            {
                qDebug()<<"using the current set of commands";

                if(!m_haveSentInitCmd)
                {
                    m_process->pty()->write(newInitCmd);
                    defineCustomFunctions();
                    m_process->pty()->write(endOfInitMarker);
                    m_haveSentInitCmd=true;
                }
            }
        }
        else
        {
            const QString message = i18n(
                "Failed to determine the version of Sage. Please check your installation and the output of 'sage -v'.");
            KMessageBox::error(nullptr, message, i18n("Cantor"));
            interrupt();
            logout();
        }
    }


    int indexOfEOI=m_outputCache.indexOf(QLatin1String("____END_OF_INIT____"));
    if(indexOfEOI!=-1&&m_outputCache.indexOf(QLatin1String(SagePrompt), indexOfEOI)!=-1)
    {
        qDebug()<<"initialized";
        //out.remove("____END_OF_INIT____");
        //out.remove(SagePrompt);
        m_isInitialized=true;
        m_waitingForPrompt=false;
        runFirstExpression();
        m_outputCache.clear();
    }

    //If we are waiting for another prompt, drop every output
    //until a prompt is found
    if(m_isInitialized&&m_waitingForPrompt)
    {
        qDebug()<<"waiting for prompt";
        if(m_outputCache.contains(QLatin1String(SagePrompt)))
            m_waitingForPrompt=false;

        m_outputCache.clear();
        return;
    }

    if(m_isInitialized)
    {
        if (!expressionQueue().isEmpty())
        {
            SageExpression* expr = static_cast<SageExpression*>(expressionQueue().first());
            expr->parseOutput(m_outputCache);
        }
        m_outputCache.clear();
    }
}

void SageSession::readStdErr()
{
    qDebug()<<"reading stdErr";
    QString out=QLatin1String(m_process->readAllStandardError());
    qDebug()<<"err: "<<out;
    if (!expressionQueue().isEmpty())
    {
        SageExpression* expr = static_cast<SageExpression*>(expressionQueue().first());
        expr->parseError(out);
    }
}

void SageSession::currentExpressionChangedStatus(Cantor::Expression::Status status)
{
    switch (status)
    {
        case Cantor::Expression::Done:
        case Cantor::Expression::Error:
            finishFirstExpression();

        default:
            break;
    }
}

void SageSession::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    if(exitStatus==QProcess::CrashExit)
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
    if(e==QProcess::FailedToStart)
    {
        changeStatus(Cantor::Session::Done);
        emit error(i18n("Failed to start Sage"));
    }
}

void SageSession::runFirstExpression()
{
    if(!expressionQueue().isEmpty())
    {
        SageExpression* expr = static_cast<SageExpression*>(expressionQueue().first());
        if (m_isInitialized)
        {
            connect(expr, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SLOT(currentExpressionChangedStatus(Cantor::Expression::Status)));

            QString command=expr->command();
            if(command.endsWith(QLatin1Char('?')) && !command.endsWith(QLatin1String("??")))
                command=QLatin1String("help(")+command.left(command.size()-1)+QLatin1Char(')');
            if(command.startsWith(QLatin1Char('?')))
                command=QLatin1String("help(")+command.mid(1)+QLatin1Char(')');
            command.append(QLatin1String("\n\n"));

            qDebug()<<"writing "<<command<<" to the process";
            expr->setStatus(Cantor::Expression::Computing);
            m_process->pty()->write(command.toUtf8());
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
            const int pid=m_process->pid();
            kill(pid, SIGINT);
#else
            ; //TODO: interrupt the process on windows
#endif
        }
        foreach (Cantor::Expression* expression, expressionQueue())
            expression->setStatus(Cantor::Expression::Interrupted);
        expressionQueue().clear();

        m_outputCache.clear();

        qDebug()<<"done interrupting";
    }

    changeStatus(Cantor::Session::Done);
}

void SageSession::sendInputToProcess(const QString& input)
{
    m_process->pty()->write(input.toUtf8());
}

void SageSession::fileCreated( const QString& path )
{
    qDebug()<<"got a file "<<path;
    if (!expressionQueue().isEmpty())
    {
        SageExpression* expr = static_cast<SageExpression*>(expressionQueue().first());
        if ( expr )
           expr->addFileResult( path );
    }
}

void SageSession::setTypesettingEnabled(bool enable)
{
    Cantor::Session::setTypesettingEnabled(enable);

    //tell the sage server to enable/disable pretty_print
    const QString cmd=QLatin1String("__cantor_enable_typesetting(%1)");
    evaluateExpression(cmd.arg(enable ? QLatin1String("true"):QLatin1String("false")), Cantor::Expression::DeleteOnFinish);
}

void SageSession::setWorksheetPath(const QString& path)
{
    m_worksheetPath = path;
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
    QString cmd=QLatin1String("def __cantor_enable_typesetting(enable):\n");
    if(m_sageVersion<VersionInfo(5, 7))
    {
        //the _ and __IP.outputcache() are needed to keep the
        // _ operator working. in modern versions of sage the __IP variable
        //has been removed
        cmd+=QLatin1String("\t sage.misc.latex.pretty_print_default(enable);_;__IP.outputcache() \n\n");
    }else if (m_sageVersion > VersionInfo(5, 7) && m_sageVersion< VersionInfo(5, 12))
    {
        cmd+=QLatin1String("\t sage.misc.latex.pretty_print_default(enable)\n\n");
    }else
    {
        cmd+=QLatin1String("\t if(enable==true):\n "\
             "\t \t %display typeset \n"\
             "\t else: \n" \
             "\t \t %display simple \n\n");
    }

    sendInputToProcess(cmd);
}

bool SageSession::updateSageVersion()
{
    QProcess get_sage_version;
    get_sage_version.setProgram(SageSettings::self()->path().toLocalFile());
    get_sage_version.setArguments(QStringList()<<QLatin1String("-v"));
    get_sage_version.start();
    if (!get_sage_version.waitForFinished(-1))
        return false;

    QString versionString = QString::fromLocal8Bit(get_sage_version.readLine());
    QRegularExpression versionExp(QLatin1String("(\\d+)\\.(\\d+)"));
    QRegularExpressionMatch version = versionExp.match(versionString);
    qDebug()<<"found version: " << version.capturedTexts();
    if(version.capturedTexts().length() == 3)
    {
        int major=version.capturedTexts().at(1).toInt();
        int minor=version.capturedTexts().at(2).toInt();
        m_sageVersion=SageSession::VersionInfo(major, minor);
        return true;
    }
    else
        return false;
}
