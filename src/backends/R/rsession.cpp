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
    Copyright (C) 2018 Alexander Semke <alexander.semke@web.de>
 */

#include "rsession.h"
#include "rexpression.h"
#include "rcompletionobject.h"
#include "rhighlighter.h"

#include <QTimer>
#include <QDebug>
#include <KProcess>

#ifndef Q_OS_WIN
#include <signal.h>
#endif

RSession::RSession(Cantor::Backend* backend) : Session(backend), m_process(nullptr), m_rServer(nullptr)
{
}

RSession::~RSession()
{
    qDebug();
    if (m_process)
        m_process->terminate();
}

void RSession::login()
{
    qDebug()<<"login";
    emit loginStarted();

    if(m_process)
        m_process->deleteLater();

    m_process = new QProcess(this);
    m_process->start(QStandardPaths::findExecutable(QLatin1String("cantor_rserver")));
    m_process->waitForStarted();
    m_process->waitForReadyRead();
    qDebug()<<m_process->readAllStandardOutput();

    m_rServer = new org::kde::Cantor::R(QString::fromLatin1("org.kde.Cantor.R-%1").arg(m_process->pid()),  QLatin1String("/"), QDBusConnection::sessionBus(), this);

    connect(m_rServer, SIGNAL(statusChanged(int)), this, SLOT(serverChangedStatus(int)));
    connect(m_rServer,SIGNAL(symbolList(const QStringList&,const QStringList&)),this,SLOT(receiveSymbols(const QStringList&,const QStringList&)));

    emit loginDone();
    qDebug()<<"login done";
}

void RSession::logout()
{
    qDebug()<<"logout";
    m_process->terminate();
}

void RSession::interrupt()
{
    const int pid = m_process->pid();
    qDebug()<<"interrupt" << pid;
    if (pid)
    {
#ifndef Q_OS_WIN
        kill(pid, SIGINT);
#else
      //TODO: interrupt the process on windows
#endif
    }
    m_expressionQueue.removeFirst();
    changeStatus(Cantor::Session::Done);
}

Cantor::Expression* RSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave)
{
    qDebug()<<"evaluating: "<<cmd;
    RExpression* expr=new RExpression(this);
    expr->setFinishingBehavior(behave);
    expr->setCommand(cmd);

    expr->evaluate();

    changeStatus(Cantor::Session::Running);

    return expr;
}

Cantor::CompletionObject* RSession::completionFor(const QString& command, int index)
{
    RCompletionObject *cmp=new RCompletionObject(command, index, this);
    connect(m_rServer,SIGNAL(completionFinished(const QString&,const QStringList&)),cmp,SLOT(receiveCompletions(const QString&,const QStringList&)));
    connect(cmp,SIGNAL(requestCompletion(const QString&)),m_rServer,SLOT(completeCommand(const QString&)));
    return cmp;
}

QSyntaxHighlighter* RSession::syntaxHighlighter(QObject* parent)
{
    RHighlighter *h=new RHighlighter(parent);
    connect(h,SIGNAL(syntaxRegExps(QVector<QRegExp>&,QVector<QRegExp>&)),this,SLOT(fillSyntaxRegExps(QVector<QRegExp>&,QVector<QRegExp>&)));
    connect(this,SIGNAL(symbolsChanged()),h,SLOT(refreshSyntaxRegExps()));
    return h;
}

void RSession::fillSyntaxRegExps(QVector<QRegExp>& v, QVector<QRegExp>& f)
{
    // WARNING: current implementation as-in-maxima is a performance hit
    // think about grouping expressions together or only fetching needed ones
    v.clear(); f.clear();

    foreach (const QString s, m_variables)
        if (!s.contains(QRegExp(QLatin1String("[^A-Za-z0-9_.]"))))
            v.append(QRegExp(QLatin1String("\\b")+s+QLatin1String("\\b")));
    foreach (const QString s, m_functions)
        if (!s.contains(QRegExp(QLatin1String("[^A-Za-z0-9_.]"))))
            f.append(QRegExp(QLatin1String("\\b")+s+QLatin1String("\\b")));
}

void RSession::receiveSymbols(const QStringList& v, const QStringList & f)
{
    m_variables=v;
    m_functions=f;

    emit symbolsChanged();
}

void RSession::queueExpression(RExpression* expr)
{
    m_expressionQueue.append(expr);

    if(status()==Cantor::Session::Done)
        QTimer::singleShot(0, this, SLOT(runNextExpression()));
}


void RSession::serverChangedStatus(int status)
{
    qDebug()<<"changed status to "<<status;
    if(status==0)
    {
        if(!m_expressionQueue.isEmpty())
        {
            RExpression* expr=m_expressionQueue.takeFirst();
            qDebug()<<"done running "<<expr<<" "<<expr->command();
        }

        if(m_expressionQueue.isEmpty())
            changeStatus(Cantor::Session::Done);
        else
            runNextExpression();
    }
    else
        changeStatus(Cantor::Session::Running);
}

void RSession::runNextExpression()
{
    if (m_expressionQueue.isEmpty())
	return;
    disconnect(m_rServer,  SIGNAL(expressionFinished(int, const QString&)),  0,  0);
    disconnect(m_rServer, SIGNAL(inputRequested(const QString&)), 0, 0);
    disconnect(m_rServer, SIGNAL(showFilesNeeded(const QStringList&)), 0, 0);
    qDebug()<<"size: "<<m_expressionQueue.size();
    RExpression* expr=m_expressionQueue.first();
    qDebug()<<"running expression: "<<expr->command();

    connect(m_rServer, SIGNAL(expressionFinished(int,  const QString &)), expr, SLOT(finished(int, const QString&)));
    connect(m_rServer, SIGNAL(inputRequested(const QString&)), expr, SIGNAL(needsAdditionalInformation(const QString&)));
    connect(m_rServer, SIGNAL(showFilesNeeded(const QStringList&)), expr, SLOT(showFilesAsResult(const QStringList&)));

    m_rServer->runCommand(expr->command());
}

void RSession::sendInputToServer(const QString& input)
{
    QString s=input;
    if(!input.endsWith(QLatin1Char('\n')))
        s+=QLatin1Char('\n');
    m_rServer->answerRequest(s);
}
