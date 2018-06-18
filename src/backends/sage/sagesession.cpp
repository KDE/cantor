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

const QByteArray SageSession::SagePrompt="sage: "; //Text, sage outputs after each command
const QByteArray SageSession::SageAlternativePrompt="....: "; //Text, sage outputs when it expects further input

//some commands that are run after login
static QByteArray initCmd="os.environ['PAGER'] = 'cat'                     \n "\
                           "sage.misc.pager.EMBEDDED_MODE = True           \n "\
                           "sage.misc.viewer.BROWSER=''                    \n "\
                           "sage.misc.viewer.viewer.png_viewer('')         \n" \
                           "sage.plot.plot3d.base.SHOW_DEFAULTS['viewer'] = 'tachyon' \n"\
                           "sage.misc.latex.EMBEDDED_MODE = True           \n "\
                           "os.environ['PAGER'] = 'cat'                    \n "\
                           "%colors nocolor                                \n "\
                           "print '____TMP_DIR____', sage.misc.misc.SAGE_TMP\n";

static QByteArray newInitCmd=
    "__CANTOR_IPYTHON_SHELL__=get_ipython()   \n "\
    "__CANTOR_IPYTHON_SHELL__.autoindent=False\n ";

static QByteArray legacyInitCmd=
    "__CANTOR_IPYTHON_SHELL__=__IPYTHON__   \n "  \
    "__CANTOR_IPYTHON_SHELL__.autoindent=False\n ";

static QByteArray endOfInitMarker="print '____END_OF_INIT____'\n ";



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

bool SageSession::VersionInfo::operator==(const SageSession::VersionInfo &other) const
{
    return m_major==other.m_major&&m_minor==other.m_minor;
}

bool SageSession::VersionInfo::operator<(const SageSession::VersionInfo &other) const
{
    return (m_major!= -1 && other.m_major==-1) ||
        ( ((m_major!=-1 && other.m_major!=-1) || (m_major==other.m_major && m_major==-1) ) && ( m_major<other.m_major||(m_major==other.m_major&&m_minor<other.m_minor) ) );
}

bool SageSession::VersionInfo::operator<=(const SageSession::VersionInfo &other) const
{
    return (*this < other)||(*this == other);
}

bool SageSession::VersionInfo::operator>(const SageSession::VersionInfo& other) const
{
    return !( (*this <= other ));
}

bool SageSession::VersionInfo::operator>=(const SageSession::VersionInfo& other) const
{
    return !( *this < other);
}

SageSession::SageSession( Cantor::Backend* backend) : Session(backend)
{
    m_isInitialized=false;
    m_haveSentInitCmd=false;
    connect( &m_dirWatch, SIGNAL( created( const QString& ) ), this, SLOT( fileCreated( const QString& ) ) );
}

SageSession::~SageSession()
{
}

void SageSession::login()
{
    qDebug()<<"login";
    emit loginStarted();

    m_process=new KPtyProcess(this);
    m_process->setProgram(SageSettings::self()->path().toLocalFile());
    m_process->setOutputChannelMode(KProcess::SeparateChannels);
    m_process->setPtyChannels(KPtyProcess::AllChannels);
    m_process->pty()->setEcho(false);

    connect(m_process->pty(), SIGNAL(readyRead()), this, SLOT(readStdOut()));
    connect(m_process, SIGNAL(readyReadStandardError()), this, SLOT(readStdErr()));
    connect(m_process, SIGNAL(finished ( int,  QProcess::ExitStatus )), this, SLOT(processFinished(int, QProcess::ExitStatus)));
    connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(reportProcessError(QProcess::ProcessError)));
    m_process->start();
    m_process->waitForStarted();

    m_process->pty()->write(initCmd);

    if(!SageSettings::self()->autorunScripts().isEmpty()){
        QString autorunScripts = SageSettings::self()->autorunScripts().join(QLatin1String("\n"));
        evaluateExpression(autorunScripts, SageExpression::DeleteOnFinish);
    }

    emit loginDone();
}

void SageSession::logout()
{
    qDebug()<<"logout";

    disconnect(m_process, SIGNAL(finished ( int,  QProcess::ExitStatus )), this, SLOT(processFinished(int, QProcess::ExitStatus)));
    m_process->pty()->write("exit\n");

    m_process->kill();
    //Run sage-cleaner to kill all the orphans
    KProcess::startDetached(SageSettings::self()->path().toLocalFile(),QStringList()<<QLatin1String("-cleaner"));

    m_expressionQueue.clear();
}

Cantor::Expression* SageSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave)
{
    qDebug()<<"evaluating: "<<cmd;
    SageExpression* expr=new SageExpression(this);
    expr->setFinishingBehavior(behave);
    expr->setCommand(cmd);
    expr->evaluate();

    return expr;
}

void SageSession::appendExpressionToQueue(SageExpression* expr)
{
    m_expressionQueue.append(expr);

    if(m_expressionQueue.size()==1)
    {
        changeStatus(Cantor::Session::Running);
        runFirstExpression();
    }
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
        QProcess get_sage_version;
        get_sage_version.setProgram(SageSettings::self()->path().toLocalFile());
        get_sage_version.setArguments(QStringList()<<QLatin1String("-v"));
        get_sage_version.start();
        get_sage_version.waitForFinished(-1);
        QString versionString = QString::fromLocal8Bit(get_sage_version.readLine());
        QRegularExpression versionExp(QLatin1String("(\\d+)\\.(\\d+)"));
        QRegularExpressionMatch version = versionExp.match(versionString);
        qDebug()<<"found version: " << version.capturedTexts();
        if(version.isValid())
        {
            int major=version.capturedTexts()[1].toInt();
            int minor=version.capturedTexts()[2].toInt();


            m_sageVersion=SageSession::VersionInfo(major, minor);

            if(m_sageVersion<=SageSession::VersionInfo(5, 7))
            {
                qDebug()<<"using an old version of sage: "<<major<<"."<<minor<<". Using the old init command";
                if(!m_haveSentInitCmd)
                {
                    m_process->pty()->write(legacyInitCmd);
                    defineCustomFunctions();
                    m_process->pty()->write(endOfInitMarker);
                    m_haveSentInitCmd=true;
                }

            }else
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
        changeStatus(Cantor::Session::Done);
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

    if(m_isInitialized&&!m_expressionQueue.isEmpty())
    {
        SageExpression* expr=m_expressionQueue.first();
        expr->parseOutput(m_outputCache);
        m_outputCache.clear();
    }
}

void SageSession::readStdErr()
{
    qDebug()<<"reading stdErr";
    QString out=QLatin1String(m_process->readAllStandardError());
    qDebug()<<"err: "<<out;
    if (!m_expressionQueue.isEmpty())
    {
        SageExpression* expr=m_expressionQueue.first();
        expr->parseError(out);
    }
}

void SageSession::currentExpressionChangedStatus(Cantor::Expression::Status status)
{
    if(status!=Cantor::Expression::Computing) //The session is ready for the next command
    {
        SageExpression* expr=m_expressionQueue.takeFirst();
        disconnect(expr, nullptr, this, nullptr);
        if(m_expressionQueue.isEmpty())
            changeStatus(Cantor::Session::Done);
        runFirstExpression();
    }

}

void SageSession::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    if(exitStatus==QProcess::CrashExit)
    {
        if(!m_expressionQueue.isEmpty())
        {
            m_expressionQueue.last()->onProcessError(i18n("The Sage process crashed while evaluating this expression"));
        }else
        {
            //We don't have an actual command. it crashed for some other reason, just show a plain error message box
            KMessageBox::error(nullptr, i18n("The Sage process crashed"), i18n("Cantor"));
        }
    }else
    {
        if(!m_expressionQueue.isEmpty())
        {
            m_expressionQueue.last()->onProcessError(i18n("The Sage process exited while evaluating this expression"));
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
    if(!m_expressionQueue.isEmpty()&&m_isInitialized)
    {
        SageExpression* expr=m_expressionQueue.first();
        connect(expr, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SLOT(currentExpressionChangedStatus(Cantor::Expression::Status)));
        QString command=expr->command();
        if(command.endsWith(QLatin1Char('?')))
            command=QLatin1String("help(")+command.left(command.size()-1)+QLatin1Char(')');
        if(command.startsWith(QLatin1Char('?')))
            command=QLatin1String("help(")+command.mid(1)+QLatin1Char(')');

        qDebug()<<"writing "<<command<<" to the process";
        m_process->pty()->write(QString(command+QLatin1String("\n\n")).toUtf8());
    }
}

void SageSession::interrupt()
{
    if(!m_expressionQueue.isEmpty())
        m_expressionQueue.first()->interrupt();
    m_expressionQueue.clear();
    changeStatus(Cantor::Session::Done);
}

void SageSession::sendSignalToProcess(int signal)
{
    qDebug()<<"sending signal....."<<signal;
    //Sage spawns several child-processes. the one we are interested in is called sage-ipython.
    //But to determine which ipython process is the one belonging to this session, we search
    //for a bash process which is child of this sessions sage process, and only kill the
    //ipython process which is child of the bash process
    // the tree looks like: m_sageprocess->bash->sage-ipython
    QString cmd=QString::fromLatin1("pkill -%1 -f -P `pgrep  -P %2 bash` .*sage-ipython.*").arg(signal).arg(m_process->pid());
    KProcess proc(this);
    proc.setShellCommand(cmd);
    proc.execute();
}

void SageSession::sendInputToProcess(const QString& input)
{
    m_process->pty()->write(input.toUtf8());
}

void SageSession::waitForNextPrompt()
{
    m_waitingForPrompt=true;
}

void SageSession::fileCreated( const QString& path )
{
    qDebug()<<"got a file "<<path;
    SageExpression* expr=m_expressionQueue.first();
    if ( expr )
        expr->addFileResult( path );
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
    //save the path to the worksheet as variable "__file__"
    //this variable is usually set by the "os" package when running a script
    //but when it is run in an interpreter (like sage server) it is not set
    const QString cmd = QLatin1String("__file__ = '%1'");
    evaluateExpression(cmd.arg(path), Cantor::Expression::DeleteOnFinish);
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

