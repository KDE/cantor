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

#include <kdebug.h>
#include <kptyprocess.h>
#include <kptydevice.h>

#include "settings.h"


const QByteArray SageSession::SagePrompt="sage: "; //Text, sage outputs after each command

//some commands that are run after login
//                           sage.plot.plot.EMBEDDED_MODE = True            \n
//                           os.environ['PAGER'] = 'cat'                    \n
//                           sage.misc.pager.EMBEDDED_MODE = True           \n

static QByteArray initCmd="os.environ['PAGER'] = 'cat'                    \n "\
                           "sage.misc.pager.EMBEDDED_MODE = True           \n "\
                           "sage.misc.viewer.BROWSER=''                    \n "\
                           "sage.misc.latex.EMBEDDED_MODE = True           \n "\
                           "sage.misc.latex.pretty_print_default(true)     \n "\
                           "print '____TMP_DIR____', sage.misc.misc.SAGE_TMP\n"\
                           "print '____END_OF_INIT____'                    \n ";

SageSession::SageSession( MathematiK::Backend* backend) : Session(backend)
{
    kDebug();
    m_isInitialized=false;

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
    m_process->start();
    m_process->pty()->write(initCmd);
}

void SageSession::logout()
{
    kDebug()<<"logout";
    interrupt();
    m_process->pty()->write("exit\n");

    m_process->deleteLater();
    //Run sage-cleaner to kill all the orphans
    KProcess::execute(SageSettings::self()->path().toLocalFile(),QStringList()<<"-cleaner");

    m_expressionQueue.clear();
}

MathematiK::Expression* SageSession::evaluateExpression(const QString& cmd)
{
    kDebug()<<"evaluating: "<<cmd;
    SageExpression* expr=new SageExpression(this);
    expr->setCommand(cmd);
    expr->evaluate();

    return expr;
}

void SageSession::appendExpressionToQueue(SageExpression* expr)
{
    m_expressionQueue.append(expr);

    if(m_expressionQueue.size()==1)
    {
        changeStatus(MathematiK::Session::Running);
        runFirstExpression();
    }
}

void SageSession::readStdOut()
{
//    if(!m_process->canReadLine()) return;

    kDebug()<<"reading stdOut";
    QString out=m_process->pty()->readAll();
    kDebug()<<"out: "<<out;

    if ( out.contains( "___TMP_DIR___" ) )
    {
        kDebug()<<"received tmp dir information";
        int index=out.indexOf("___TMP_DIR___" )+14;
        m_tmpPath=out.mid( index, index-out.indexOf('\n',index ) ).trimmed();
        kDebug()<<"tmp path: "<<m_tmpPath;

        m_dirWatch.addDir( m_tmpPath, KDirWatch::WatchFiles );
    }

    if(out.contains("____END_OF_INIT____"))
    {
        kDebug()<<"initialized";
        out.remove("____END_OF_INIT____");
        out.remove(SagePrompt);
        m_isInitialized=true;
        runFirstExpression();
        changeStatus(MathematiK::Session::Done);
        emit ready();
    }

    if(!m_expressionQueue.isEmpty())
    {
        SageExpression* expr=m_expressionQueue.first();
        expr->parseOutput(out);
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

void SageSession::currentExpressionChangedStatus(MathematiK::Expression::Status status)
{
    if(status!=MathematiK::Expression::Computing) //The session is ready for the next command
    {
        m_expressionQueue.removeFirst();
        if(m_expressionQueue.isEmpty())
            changeStatus(MathematiK::Session::Done);
        runFirstExpression();
    }

}

void SageSession::runFirstExpression()
{
    if(!m_expressionQueue.isEmpty()&&m_isInitialized)
    {
        SageExpression* expr=m_expressionQueue.first();
        connect(expr, SIGNAL(statusChanged(MathematiK::Expression::Status)), this, SLOT(currentExpressionChangedStatus(MathematiK::Expression::Status)));
        QString command=expr->command();
        if(command.endsWith('?'))
            command=("help("+command.left(command.size()-1)+")");
        if(command.startsWith('?'))
            command=("help("+command.mid(1)+")");

        kDebug()<<"writing "<<command+"\n"<<" to the process";
        m_process->pty()->write((command+"\n").toLatin1());
    }
}

void SageSession::interrupt()
{
    if(!m_expressionQueue.isEmpty())
        m_expressionQueue.first()->interrupt();
    m_expressionQueue.clear();
    changeStatus(MathematiK::Session::Done);
}

void SageSession::sendSignalToProcess(int signal)
{
    Q_UNUSED(signal)
    kDebug()<<"sending signal.....";
    //Sage spawns some child-processe. the one we are interested is called sage-ipython.
    //But to determine which ipython process is the one belonging to this session, we search
    //for a bash process which is child of this sessions sage process, and only kill the
    //ipython process which is child of the bash process
    // the tree looks like: m_sageprocess->bash->sage-ipython
    QString cmd=QString("pkill -2 -P `pgrep  -P %1 bash` sage-ipython").arg(m_process->pid());
    KProcess proc(this);
    proc.setShellCommand(cmd);
    proc.execute();
}

void SageSession::fileCreated( const QString& path )
{
    SageExpression* expr=m_expressionQueue.first();
    if ( expr )
        expr->addFileResult( path );
}

void SageSession::setTypesettingEnabled(bool enable)
{
    MathematiK::Session::setTypesettingEnabled(enable);

    if (enable)
        evaluateExpression("sage.misc.latex.pretty_print_default(true)");
    else
        evaluateExpression("sage.misc.latex.pretty_print_default(false)");
}

MathematiK::Expression* SageSession::contextHelp(const QString& command)
{
    bool t=isTypesettingEnabled();
    if(t)
        setTypesettingEnabled(false);

    MathematiK::Expression* e=evaluateExpression("dir("+command+")");

    if(t)
        setTypesettingEnabled(true);

    return e;
}

#include "sagesession.moc"
