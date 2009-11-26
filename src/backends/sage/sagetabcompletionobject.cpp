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
#include <QStack>

SageTabCompletionObject::SageTabCompletionObject(const QString& command, SageSession* session) : Cantor::TabCompletionObject(command, session)
{
    m_expression=0;

    //Only use the completion for the last command part between end and opening bracket or ;
    QString cmd=command;
    int semIndex=cmd.lastIndexOf(';')+1;
    cmd=cmd.mid(semIndex);

    //Find last unmatched open bracket
    QStack<int> brIndex;
    QPair<int,int> lastClosedBracket=QPair<int, int>(0, 0);
    for(int i=0;i<cmd.length();i++)
    {
        if(cmd[i]=='(')
        {
            brIndex.push(i);
        }

        if(cmd[i]==')')
        {
            const int index=brIndex.pop();
            lastClosedBracket.first=index;
            lastClosedBracket.second=i;
         }
    }

    //remove code before the last unmatched bracket
    if(!brIndex.isEmpty())
    {
        const int index=brIndex.pop()+1;
        cmd=cmd.mid(index);
        lastClosedBracket.first-=index;
        lastClosedBracket.second-=index;
    }

    //remove code before the outermost block, keeping the part between the last +-*/ and the opening bracket
    {
        const int index=cmd.lastIndexOf(QRegExp("[=\\+\\-\\*\\/\\<\\>]"), lastClosedBracket.first)+1;
        cmd=cmd.mid(index);
        lastClosedBracket.second-=index;
    }

    //only keep code between the last sign, outside of a ()-block, and the end
    {
        const int index=cmd.lastIndexOf(QRegExp("[=\\+\\-\\*\\/\\<\\>]"))+1;
        if(index>=lastClosedBracket.second)
            cmd=cmd.mid(index);
    }

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



