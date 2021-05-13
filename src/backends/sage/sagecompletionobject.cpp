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
        emit fetchingDone();
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

    QStringList tmp=txt.split(QLatin1Char(','));
    QStringList completions;

    foreach(QString c, tmp) // krazy:exclude=foreach
    {
        c=c.trimmed();
        c.chop(1);
        completions<<c.mid(1);
    }

    completions << SageKeywords::instance()->keywords();
    setCompletions(completions);

    emit fetchingDone();
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

    QStringList tmp=txt.split(QLatin1Char(','));
    QStringList completions;

    foreach(QString c, tmp) // krazy:exclude=foreach
    {
        c=c.trimmed();
        c.chop(1);
        completions<<c.mid(1);
    }

    completions << SageKeywords::instance()->keywords();
    setCompletions(completions);

    emit fetchingDone();
}


void SageCompletionObject::fetchIdentifierType()
{
    if (SageKeywords::instance()->keywords().contains(identifier()))
    {
        emit fetchingTypeDone(KeywordType);
        return;
    }

    if (session()->status() != Cantor::Session::Done)
    {
        if (SageKeywords::instance()->functions().contains(identifier()))
            emit fetchingTypeDone(FunctionType);
        else if (SageKeywords::instance()->variables().contains(identifier()))
            emit fetchingTypeDone(VariableType);
        else
            emit fetchingTypeDone(UnknownType);
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
            emit fetchingTypeDone(UnknownType);
            break;

        case Cantor::Expression::Interrupted:
            qDebug() << "SageCompletionObject was interrupted";
            emit fetchingTypeDone(UnknownType);
            break;

        case Cantor::Expression::Done:
        {
            Cantor::Result* result = m_expression->result();
            if (result)
            {
                QString res = result->data().toString();
                if (res.contains(QLatin1String("function")) || res.contains(QLatin1String("method")))
                    emit fetchingTypeDone(FunctionType);
                else
                    emit fetchingTypeDone(VariableType);
            }
            else
                emit fetchingTypeDone(UnknownType);
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
