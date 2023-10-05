/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "sagecompletionobject.h"

#include "sagesession.h"
#include "sagekeywords.h"
#include "textresult.h"

#include <QDebug>
#include <QStack>

using namespace Cantor;

SageCompletionObject::SageCompletionObject(const QString& command, int index, SageSession* session) : Cantor::CompletionObject(session)
{
    setLine(command, index);
    m_expression=nullptr;
}

SageCompletionObject::~SageCompletionObject()
{
    if(m_expression)
        m_expression->setFinishingBehavior(Expression::DeleteOnFinish);
}

void SageCompletionObject::fetchCompletions()
{
    if (session()->status() != Cantor::Session::Done)
    {
        QStringList allCompletions;

        allCompletions << SageKeywords::instance()->keywords();
        allCompletions << SageKeywords::instance()->functions();
        allCompletions << SageKeywords::instance()->variables();

        setCompletions(allCompletions);
        Q_EMIT fetchingDone();
    }
    else
    {
        if (m_expression)
            return;

        //cache the value of the "_" variable into __hist_tmp__, so we can restore the previous result
        //after complete() was evaluated
        const QString& cmd = QLatin1String("__hist_tmp__=_; sage.interfaces.tab_completion.completions(\"")+command()+QLatin1String("\",globals());_=__hist_tmp__");
        m_expression=session()->evaluateExpression(cmd, Cantor::Expression::FinishingBehavior::DoNotDelete, true);
        connect(m_expression, &Cantor::Expression::gotResult, this, &SageCompletionObject::extractCompletions);
    }
}

void SageCompletionObject::extractCompletions()
{
  SageSession* s=qobject_cast<SageSession*>(session());
  if(s&&s->sageVersion()<SageSession::VersionInfo(5, 7))
    extractCompletionsLegacy();
  else
    extractCompletionsNew();
}

void SageCompletionObject::extractCompletionsNew()
{
    Cantor::Result* res=m_expression->result();
    m_expression->deleteLater();
    m_expression=nullptr;

    if(!res || !(res->type()==Cantor::TextResult::Type))
    {
        qDebug()<<"something went wrong fetching tab completion";
        fetchingDone();
        return;
    }

    //the result looks like "['comp1', 'comp2']" parse it

    QString txt=res->data().toString().trimmed();
    txt=txt.mid(1); //remove [
    txt.chop(1); //remove ]

    qDebug()<<"completion string: "<<txt;

    const QStringList tmp = txt.split(QLatin1Char(','));
    QStringList completions;

    for(QString c : tmp) {
        c=c.trimmed();
        c.chop(1);
        completions<<c.mid(1);
    }

    completions << SageKeywords::instance()->keywords();
    setCompletions(completions);

    Q_EMIT fetchingDone();
}

void SageCompletionObject::extractCompletionsLegacy()
{
    Cantor::Result* res=m_expression->result();
    m_expression->deleteLater();
    m_expression=nullptr;

    if(!res || !(res->type()==Cantor::TextResult::Type))
    {
        qDebug()<<"something went wrong fetching tab completion";
        fetchingDone();
        return;
    }

    //the result looks like "['comp1', 'comp2']" parse it
    QString txt=res->data().toString().trimmed();
    txt=txt.mid(1); //remove [
    txt.chop(1); //remove ]

    const QStringList tmp = txt.split(QLatin1Char(','));
    QStringList completions;

    for (QString c : tmp)
    {
        c=c.trimmed();
        c.chop(1);
        completions<<c.mid(1);
    }

    completions << SageKeywords::instance()->keywords();
    setCompletions(completions);

    Q_EMIT fetchingDone();
}


void SageCompletionObject::fetchIdentifierType()
{
    if (SageKeywords::instance()->keywords().contains(identifier()))
    {
        Q_EMIT fetchingTypeDone(KeywordType);
        return;
    }

    if (session()->status() != Cantor::Session::Done)
    {
        if (SageKeywords::instance()->functions().contains(identifier()))
            Q_EMIT fetchingTypeDone(FunctionType);
        else if (SageKeywords::instance()->variables().contains(identifier()))
            Q_EMIT fetchingTypeDone(VariableType);
        else
            Q_EMIT fetchingTypeDone(UnknownType);
    }
    else
    {
        if (m_expression)
            return;

        QString expr = QString::fromLatin1("__cantor_internal__ = _; type(%1); _ = __cantor_internal__").arg(identifier());
        m_expression = session()->evaluateExpression(expr, Cantor::Expression::FinishingBehavior::DoNotDelete, true);
        connect(m_expression, &Cantor::Expression::statusChanged, this, &SageCompletionObject::extractIdentifierType);
    }
}

void SageCompletionObject::extractIdentifierType(Cantor::Expression::Status status)
{
    switch(status)
    {
        case Cantor::Expression::Error:
            qDebug() << "Error with SageCompletionObject" << m_expression->errorMessage();
            Q_EMIT fetchingTypeDone(UnknownType);
            break;

        case Cantor::Expression::Interrupted:
            qDebug() << "SageCompletionObject was interrupted";
            Q_EMIT fetchingTypeDone(UnknownType);
            break;

        case Cantor::Expression::Done:
        {
            Cantor::Result* result = m_expression->result();
            if (result)
            {
                QString res = result->data().toString();
                if (res.contains(QLatin1String("function")) || res.contains(QLatin1String("method")))
                    Q_EMIT fetchingTypeDone(FunctionType);
                else
                    Q_EMIT fetchingTypeDone(VariableType);
            }
            else
                Q_EMIT fetchingTypeDone(UnknownType);
            break;
        }

        default:
            return;
    }

    m_expression->deleteLater();
    m_expression = nullptr;
}

bool SageCompletionObject::mayIdentifierContain(QChar c) const
{
    return c.isLetter() || c.isDigit() || c == QLatin1Char('_') || c == QLatin1Char('.');
}

bool SageCompletionObject::mayIdentifierBeginWith(QChar c) const
{
    return c.isLetter() || c.isDigit() || c == QLatin1Char('_');
}
