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
    bool t=session()->isTypesettingEnabled();
    if(t)
        session()->setTypesettingEnabled(false);

    //cache the value of the "_" variable into __hist_tmp__, so we can restore the previous result
    //after complete() was evaluated
    m_expression=session()->evaluateExpression("__hist_tmp__=_; __IPYTHON__.complete(\""+command()+"\");_=__hist_tmp__");
    connect(m_expression, SIGNAL(gotResult()), this, SLOT(fetchingDone()));

    if(t)
        session()->setTypesettingEnabled(true);
}

void SageCompletionObject::fetchingDone()
{
    Cantor::Result* res=m_expression->result();
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

    setCompletions(completions);

    m_expression->deleteLater();
    m_expression=0;

    emit done();
}



