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
#include <QProcess>
#include <QStandardPaths>


#include <signal.h>

RSession::RSession( Cantor::Backend* backend) : Session(backend),
  m_Process(0),
  m_CurrentExpression(0)
{
}

RSession::~RSession()
{
    qDebug();
    m_Process->terminate();
}

void RSession::login()
{
    qDebug()<<"login";


    m_Process=new QProcess(this);
    m_Process->setProcessChannelMode(QProcess::SeparateChannels);

    QString path = QStandardPaths::findExecutable(QLatin1String("R"));
    qDebug() << path;

    if(QStandardPaths::findExecutable(QLatin1String("R")).isEmpty())
        qDebug() << "Could not find the R exe" << endl;



    m_Process->setProgram(QStandardPaths::findExecutable(QLatin1String("R")));
    QStringList args;
    /*
     * interactive - forcing an interactive session
     * quiet - don't print the startup message
     * no-save -  don't save the workspace after the end of session(when users quits). Should the user  be allowed to save the session?
     */
    args.append(QLatin1String("--interactive"));
    args.append(QLatin1String("--quiet"));
    args.append(QLatin1String("--no-save"));
    m_Process->setArguments(args);

    connect(m_Process, SIGNAL(readyReadStandardOutput()), this, SLOT(readOutput()));
    connect(m_Process, SIGNAL(readyReadStandardError()), this, SLOT(readError()));
    connect(m_Process, SIGNAL(started()), this, SLOT(processStarted()));


    m_Process->start();

}


void RSession::readOutput()
{
    while(m_Process->bytesAvailable()) {
        m_Output.append(QString::fromLocal8Bit(m_Process->readLine()));
    }

    if(m_CurrentExpression && !m_Output.isEmpty() && m_Output.trimmed().endsWith(QLatin1String(">"))) {
       m_CurrentExpression->parseOutput(m_Output);
       m_Output.clear();
    }
}

void RSession::readError()
{
    m_Error = QString::fromLocal8Bit(m_Process->readAllStandardError());
    m_CurrentExpression->parseError(m_Error);
    m_Error.clear();
}

void RSession::processStarted()
{
    qDebug() << m_Process->program() << " with pid " <<  m_Process->processId() << " started successfully " << endl;
    emit ready();
}

void RSession::logout()
{
    qDebug()<<"logout" << endl;

    // this happens if the user gives quit command
    if(status() ==  Cantor::Session::Running)
        changeStatus(Cantor::Session::Done);

    if(m_Process && m_Process->state() == QProcess::Running){
        qDebug () << " Killing the process ";
        m_Process->kill();
    }
}

void RSession::interrupt()
{
    qDebug()<<"interrupt" << m_Process->pid();
    changeStatus(Cantor::Session::Done);
}

Cantor::Expression* RSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave)
{
    qDebug()<<"evaluating: "<<cmd;
    changeStatus(Cantor::Session::Running);    
    RExpression* expr=new RExpression(this);
    expr->setFinishingBehavior(behave);
    expr->setCommand(cmd);

    /* start evaluating only if the process is in running state
     * imo this is a late check, can this be done somewhere before ?
    */
    if(m_Process && m_Process->state() == QProcess::NotRunning){
        changeStatus(Cantor::Session::Done);
        return expr;
    }

    expr->evaluate();

    return expr;
}

void RSession::runExpression(RExpression* expr)
{
    m_CurrentExpression = expr;
    connect(m_CurrentExpression, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SLOT(currentExpressionStatusChanged(Cantor::Expression::Status)));

    QString currentCmd = m_CurrentExpression->command();
    currentCmd.trimmed();
    currentCmd += QLatin1String("\n");

    m_Process->write(currentCmd.toLocal8Bit());


}

void RSession::currentExpressionStatusChanged(Cantor::Expression::Status status)
{
    switch (status) {

        case Cantor::Expression::Computing:
            break;
        case Cantor::Expression::Interrupted:
            changeStatus(Cantor::Session::Done);
            break;
        case Cantor::Expression::Done:
        case Cantor::Expression::Error:
            changeStatus(Cantor::Session::Done);

    }
}
Cantor::CompletionObject* RSession::completionFor(const QString& command, int index)
{
    /*
     * since we won't be using rserver anymore, we will have to find a different method to make completion work
     * can we use R's command line interface for this ?
    */

//    RCompletionObject *cmp=new RCompletionObject(command, index, this);
//    connect(m_rServer,SIGNAL(completionFinished(const QString&,const QStringList&)),cmp,SLOT(receiveCompletions(const QString&,const QStringList&)));
//    connect(cmp,SIGNAL(requestCompletion(const QString&)),m_rServer,SLOT(completeCommand(const QString&)));
//    return cmp;
    return 0;
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


#include "rsession.moc"
