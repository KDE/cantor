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
    Copyright (C) 2009-2012 Alexander Rieder <alexanderrieder@gmail.com>
 */

#include "maximasyntaxhelpobject.h"
#include "maximakeywords.h"

#include "maximasession.h"
#include "maximaexpression.h"

#include "result.h"

#include <QDebug>

MaximaSyntaxHelpObject::MaximaSyntaxHelpObject(const QString& cmd, MaximaSession* session) : Cantor::SyntaxHelpObject(cmd, session)
{
    m_expression=nullptr;
}

void MaximaSyntaxHelpObject::fetchInformation()
{
    bool isValid=false;
    for (const QString& func : MaximaKeywords::instance()->functions())
    {
        if(command()==func)
        {
            isValid=true;
            break;
        }
    }

    if(isValid)
    {
        if (session()->status() != Cantor::Session::Disable)
        {
            if (m_expression)
                return;

            //use the lisp command, instead of directly calling the
            //maxima function "describe" to avoid generating a new
            //output label that would mess up history
            QString cmd=QLatin1String(":lisp(cl-info::info-exact \"%1\")");

            m_expression=session()->evaluateExpression(cmd.arg(command()), Cantor::Expression::FinishingBehavior::DoNotDelete, true);

            connect(m_expression, &Cantor::Expression::statusChanged, this, &MaximaSyntaxHelpObject::expressionChangedStatus);
        }
        else
            // We can't get function's detailed description, because session not login yet, so do nothing
            emit done();

    }else
    {
        qDebug()<<"invalid syntax request";
        emit done();
    }
}

void MaximaSyntaxHelpObject::expressionChangedStatus(Cantor::Expression::Status status)
{
    switch(status)
    {
        case Cantor::Expression::Done:
        {
            qDebug()<<"expr done";
            QString text=m_expression->result()->toHtml();
            QStringList lines=text.split(QLatin1Char('\n'));

            QString syntax;
            for (QString line : lines)
            {
                if(line.endsWith(QLatin1Char('\r')))
                    line.chop(1);
                if(line.startsWith(QLatin1String("-- Function:")))
                {
                    line.remove(QLatin1String("-- Function:"));
                    line.remove(QLatin1String("<br/>"));
                }
                syntax+=line;
                qDebug() << "line: " << line;
            }

            setHtml(QLatin1String("<p style='white-space:pre'>")+syntax+QLatin1String("</p>"));
            emit done();

            m_expression->deleteLater();
            m_expression=nullptr;
            break;
        }
        case Cantor::Expression::Error:
        {
            qWarning() << "syntax object error" << m_expression->result()->toHtml();
            emit done();

            m_expression->deleteLater();
            m_expression=nullptr;
            break;
        }
        default:
            break;
    }
}
