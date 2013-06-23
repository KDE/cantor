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

#include <kdebug.h>
#include <kptyprocess.h>
#include <kptydevice.h>
#include <klocale.h>
#include <kmessagebox.h>
#include "settings.h"
#include "sagehighlighter.h"

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
    "__CANTOR_IPYTHON_SHELL__.autoindent=False\n "\
    "print '____END_OF_INIT____'              \n ";

static QByteArray legacyInitCmd=
    "__CANTOR_IPYTHON_SHELL__=__IPYTHON__   \n "  \
    "__CANTOR_IPYTHON_SHELL__.autoindent=False\n "\
    "print '____END_OF_INIT____'              \n ";


SageSession::SageSession( Cantor::Backend* backend) : Session(backend)
{
    kDebug();
    m_isInitialized=false;
    m_inLegacyMode=false;
    m_haveSentInitCmd=false;
    connect( &m_dirWatch, SIGNAL( created( const QString& ) ), this, SLOT( fileCreated( const QString& ) ) );
}

SageSession::~SageSession()
{
    kDebug();
}

void SageSession::login()
{
    kDebug()<<"login";
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

    m_process->pty()->write(initCmd);
}

void SageSession::logout()
{
    kDebug()<<"logout";
    interrupt();
    disconnect(m_process, SIGNAL(finished ( int,  QProcess::ExitStatus )), this, SLOT(processFinished(int, QProcess::ExitStatus)));
    m_process->pty()->write("exit\n");

    m_process->deleteLater();
    //Run sage-cleaner to kill all the orphans
    KProcess::startDetached(SageSettings::self()->path().toLocalFile(),QStringList()<<"-cleaner");

    m_expressionQueue.clear();
}

Cantor::Expression* SageSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave)
{
    kDebug()<<"evaluating: "<<cmd;
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
    m_outputCache.append(m_process->pty()->readAll());
    kDebug()<<"out: "<<m_outputCache;

    if ( m_outputCache.contains( "___TMP_DIR___" ) )
    {
        int index=m_outputCache.indexOf("___TMP_DIR___" )+14;
        int endIndex=m_outputCache.indexOf("\n", index);

        if(endIndex==-1)
            m_tmpPath=m_outputCache.mid( index ).trimmed();
        else
            m_tmpPath=m_outputCache.mid( index, endIndex-index ).trimmed();

        kDebug()<<"tmp path: "<<m_tmpPath;

        m_dirWatch.addDir( m_tmpPath, KDirWatch::WatchFiles );     
    }

    if(!m_isInitialized)
    {
        //try to guess the version of sage to determine
        //if we have to use the legacy commands or not.
        QRegExp versionExp("Sage\\s+Version\\s+(\\d+)\\.(\\d+)");
        int index=versionExp.indexIn(m_outputCache);
        if(index!=-1)
        {
            QStringList version=versionExp.capturedTexts();
            kDebug()<<"found version: "<<version;
            if(version.size()>2)
            {
                int major=version[1].toInt();
                int minor=version[2].toInt();
            
                if(major<=5&&minor<=7)
                {
                    m_inLegacyMode=true;
                    kDebug()<<"using an old version of sage: "<<major<<"."<<minor<<". switching to legacy mode";
                    if(!m_haveSentInitCmd)
                    {
                        m_process->pty()->write(legacyInitCmd);
                        m_haveSentInitCmd=true;
                    }

                }else
                {
                    kDebug()<<"using the current set of commands";
                    if(!m_haveSentInitCmd)
                    {
                        m_process->pty()->write(newInitCmd);
                        m_haveSentInitCmd=true;
                    }
                }
            }
        }
    }
    

    int indexOfEOI=m_outputCache.indexOf("____END_OF_INIT____");
    if(indexOfEOI!=-1&&m_outputCache.indexOf(SagePrompt, indexOfEOI)!=-1)
    {
        kDebug()<<"initialized";
        //out.remove("____END_OF_INIT____");
        //out.remove(SagePrompt);
        m_isInitialized=true;
        m_waitingForPrompt=false;
        runFirstExpression();
        changeStatus(Cantor::Session::Done);
        emit ready();
        m_outputCache.clear();
    }

    //If we are waiting for another prompt, drop every output
    //until a prompt is found
    if(m_isInitialized&&m_waitingForPrompt)
    {
        kDebug()<<"waiting for prompt";
        if(m_outputCache.contains(SagePrompt))
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
    kDebug()<<"reading stdErr";
    QString out=m_process->readAllStandardError();
    kDebug()<<"err: "<<out;
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
        disconnect(expr, 0, this, 0);
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
            KMessageBox::error(0, i18n("The Sage process crashed"), i18n("Cantor"));
        }
    }else
    {
        if(!m_expressionQueue.isEmpty())
        {
            m_expressionQueue.last()->onProcessError(i18n("The Sage process exited while evaluating this expression"));
        }else
        {
            //We don't have an actual command. it crashed for some other reason, just show a plain error message box
            KMessageBox::error(0, i18n("The Sage process exited"), i18n("Cantor"));
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
        if(command.endsWith('?'))
            command=("help("+command.left(command.size()-1)+')');
        if(command.startsWith('?'))
            command=("help("+command.mid(1)+')');

        kDebug()<<"writing "<<command<<" to the process";
        m_process->pty()->write(QString(command+"\n\n").toUtf8());
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
    kDebug()<<"sending signal....."<<signal;
    //Sage spawns several child-processes. the one we are interested in is called sage-ipython.
    //But to determine which ipython process is the one belonging to this session, we search
    //for a bash process which is child of this sessions sage process, and only kill the
    //ipython process which is child of the bash process
    // the tree looks like: m_sageprocess->bash->sage-ipython
    QString cmd=QString("pkill -%1 -f -P `pgrep  -P %2 bash` .*sage-ipython.*").arg(signal).arg(m_process->pid());
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
    kDebug()<<"got a file "<<path;
    SageExpression* expr=m_expressionQueue.first();
    if ( expr )
        expr->addFileResult( path );
}

void SageSession::setTypesettingEnabled(bool enable)
{
    Cantor::Session::setTypesettingEnabled(enable);
    //tell the sage server to enable/disable pretty_print
    if(inLegacyMode())
    {
        //the _ and __IP.outputcache() are needed to keep the
        // _ operator working. in modern versions of sage the __IP variable
        //has been removed
        if (enable)
            evaluateExpression("sage.misc.latex.pretty_print_default(true);_;__IP.outputcache()", Cantor::Expression::DeleteOnFinish);
        else
            evaluateExpression("sage.misc.latex.pretty_print_default(false);_;__IP.outputcache()", Cantor::Expression::DeleteOnFinish);
    }else
    {
        if (enable)
            evaluateExpression("sage.misc.latex.pretty_print_default(true)", Cantor::Expression::DeleteOnFinish);
        else
            evaluateExpression("sage.misc.latex.pretty_print_default(false)", Cantor::Expression::DeleteOnFinish);
    }
}

Cantor::CompletionObject* SageSession::completionFor(const QString& command, int index)
{
    return new SageCompletionObject(command, index, this);
}

QSyntaxHighlighter* SageSession::syntaxHighlighter(QObject* parent)
{
    return new SageHighlighter(parent);
}

bool SageSession::inLegacyMode()
{
  return m_inLegacyMode;
}

#include "sagesession.moc"
