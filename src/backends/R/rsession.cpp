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

#include "rsession.h"
#include "rexpression.h"
#include "rcompletionobject.h"
#include "rhighlighter.h"

#include <QTimer>
#include <QDebug>
#include <KProcess>
#include <KStandardDirs>

#include <signal.h>

RSession::RSession( Cantor::Backend* backend) : Session(backend)
{
    qDebug();
    m_rProcess=0;
}

RSession::~RSession()
{
    qDebug();
    m_rProcess->terminate();
}

void RSession::login()
{
    qDebug()<<"login";
    if(m_rProcess)
        m_rProcess->deleteLater();
    m_rProcess=new KProcess(this);
    m_rProcess->setOutputChannelMode(KProcess::ForwardedChannels);

    (*m_rProcess)<<KStandardDirs::findExe( QLatin1String("cantor_rserver") );

    m_rProcess->start();

    m_rServer=new org::kde::Cantor::R(QString::fromLatin1("org.kde.cantor_rserver-%1").arg(m_rProcess->pid()),  QLatin1String("/R"), QDBusConnection::sessionBus(), this);

    connect(m_rServer, SIGNAL(statusChanged(int)), this, SLOT(serverChangedStatus(int)));
    connect(m_rServer,SIGNAL(symbolList(const QStringList&,const QStringList&)),this,SLOT(receiveSymbols(const QStringList&,const QStringList&)));

    changeStatus(Cantor::Session::Done);

    connect(m_rServer, SIGNAL(ready()), this, SIGNAL(ready()));
}

void RSession::logout()
{
    qDebug()<<"logout";
    m_rProcess->terminate();
}

void RSession::interrupt()
{
    qDebug()<<"interrupt" << m_rProcess->pid();
    if (m_rProcess->pid())
	kill(m_rProcess->pid(), 2);
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

#include "rsession.moc"
