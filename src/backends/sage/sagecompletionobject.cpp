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

#include <kdebug.h>
#include <QStack>

SageCompletionObject::SageCompletionObject(const QString& command, int index, SageSession* session) : Cantor::CompletionObject(session)
{
    setLine(command, index);
    m_expression=0;
}

SageCompletionObject::~SageCompletionObject()
{
    if(m_expression)
    {
        m_expression->interrupt();
        m_expression->deleteLater();
    }
}

void SageCompletionObject::fetchCompletions()
{
    if (m_expression)
	return;
    bool t=session()->isTypesettingEnabled();
    if(t)
        session()->setTypesettingEnabled(false);

    //cache the value of the "_" variable into __hist_tmp__, so we can restore the previous result
    //after complete() was evaluated
    m_expression=session()->evaluateExpression("__hist_tmp__=_; __CANTOR_IPYTHON_SHELL__.complete(\""+command()+"\");_=__hist_tmp__");
    connect(m_expression, SIGNAL(gotResult()), this, 
	    SLOT(extractCompletions()));

    if(t)
        session()->setTypesettingEnabled(true);
}

void SageCompletionObject::extractCompletions()
{
  SageSession* s=qobject_cast<SageSession*>(session());
  if(s&&s->inLegacyMode())
    extractCompletionsLegacy();
  else
    extractCompletionsNew();
}

void SageCompletionObject::extractCompletionsNew()
{
    Cantor::Result* res=m_expression->result();
    m_expression->deleteLater();
    m_expression=0;

    if(!res||!res->type()==Cantor::TextResult::Type)
    {
        kDebug()<<"something went wrong fetching tab completion";
        return;
    }
    
    //the result looks like "['comp1', 'comp2']" parse it

    //for sage version 5.7 this looks like
    //('s1', ['comp1','comp2']) where s1 is the string we called complete with

    QString txt=res->toHtml().trimmed();
    txt.remove("<br/>");
    txt=txt.mid(txt.indexOf(command())+command().length()+2).trimmed();
    txt=txt.mid(1); //remove [
    txt.chop(2); //remove ]

    kDebug()<<"completion string: "<<txt;

    QStringList tmp=txt.split(',');
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
    m_expression=0;

    if(!res||!res->type()==Cantor::TextResult::Type)
    {
        kDebug()<<"something went wrong fetching tab completion";
        return;
    }

    //the result looks like "['comp1', 'comp2']" parse it
    QString txt=res->toHtml().trimmed();
    txt=txt.mid(1); //remove [
    txt.chop(1); //remove ]

    QStringList tmp=txt.split(',');
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
    if (m_expression)
	return;
    if (SageKeywords::instance()->keywords().contains(identifier())) {
	emit fetchingTypeDone(KeywordType);
	return;
    }
    QString expr = QString("__cantor_internal__ = _; type(%1); _ = __cantor_internal__").arg(identifier());
    m_expression = session()->evaluateExpression(expr);
    connect (m_expression, SIGNAL(statusChanged(Cantor::Expression::Status)), SLOT(extractIdentifierType()));
}

void SageCompletionObject::extractIdentifierType()
{
    if (m_expression->status() != Cantor::Expression::Done)
    {
	m_expression->deleteLater();
	m_expression = 0;
        return;
    }
    Cantor::Result* result = m_expression->result();
    m_expression->deleteLater();
    m_expression = 0;
    if (!result)
	return;

    QString res = result->toHtml();
    if (res.contains("function") || res.contains("method"))
	emit fetchingTypeDone(FunctionType);
    else
	emit fetchingTypeDone(VariableType);
}

bool SageCompletionObject::mayIdentifierContain(QChar c) const
{
    return c.isLetter() || c.isDigit() || c == '_' || c == '.';
}

bool SageCompletionObject::mayIdentifierBeginWith(QChar c) const
{
    return c.isLetter() || c.isDigit() || c == '_';
}
