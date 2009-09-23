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

#include "sagetabcompletionobject.h"

#include "sagesession.h"
#include "textresult.h"

#include <kdebug.h>

SageTabCompletionObject::SageTabCompletionObject(const QString& command, SageSession* session) : MathematiK::TabCompletionObject(command, session)
{
    m_expression=0;

    //Only use the completion for the last command part between end and opening bracket or ;
    QString cmd=command;
    int brIndex=cmd.lastIndexOf('(')+1;
    int semIndex=cmd.lastIndexOf(';')+1;
    cmd=cmd.mid(qMax(brIndex, semIndex));

    setCommand(cmd);
}

SageTabCompletionObject::~SageTabCompletionObject()
{
    if(m_expression)
    {
        m_expression->interrupt();
        m_expression->deleteLater();
    }
}

void SageTabCompletionObject::fetchCompletions()
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

void SageTabCompletionObject::fetchingDone()
{
    MathematiK::Result* res=m_expression->result();
    if(!res||!res->type()==MathematiK::TextResult::Type)
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



