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

#include "maximasyntaxhelpobject.h"

#include "maximasession.h"
#include "maximaexpression.h"

#include "result.h"

#include <kdebug.h>

MaximaSyntaxHelpObject::MaximaSyntaxHelpObject(const QString& cmd, MaximaSession* session) : Cantor::SyntaxHelpObject(cmd, session)
{
    m_expression=0;
}

MaximaSyntaxHelpObject::~MaximaSyntaxHelpObject()
{

}

void MaximaSyntaxHelpObject::fetchInformation()
{
    if(m_expression)
    {
        m_expression->setFinishingBehavior(Cantor::Expression::DeleteOnFinish);
    }

    m_expression=static_cast<MaximaSession*>(session())->evaluateHelperExpression(QString("describe(%1);").arg(command()));
    connect(m_expression, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SLOT(expressionChangedStatus(Cantor::Expression::Status)));
}

void MaximaSyntaxHelpObject::expressionChangedStatus(Cantor::Expression::Status status)
{
    if(status==Cantor::Expression::Done)
    {
        kDebug()<<"expr done";
        QString text=m_expression->result()->toHtml();
        QStringList lines=text.split('\n');

        QString completions;
        foreach(QString line, lines)
        {
            line=line.trimmed();
            if(line.startsWith("-- Function:"))
            {
                line.remove("-- Function:");
                completions+=line.trimmed()+'\n';
            }else
                 break;
        }
        setHtml(completions);
        emit done();

        m_expression->deleteLater();
        m_expression=0;
    }else
    {
        kDebug()<<"not done: "<<status;
    }
}
