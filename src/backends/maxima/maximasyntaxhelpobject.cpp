/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009-2012 Alexander Rieder <alexanderrieder@gmail.com>
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
            Q_EMIT done();

    }else
    {
        qDebug()<<"invalid syntax request";
        Q_EMIT done();
    }
}

void MaximaSyntaxHelpObject::expressionChangedStatus(Cantor::Expression::Status status)
{
    switch(status)
    {
        case Cantor::Expression::Done:
        {
            qDebug()<<"expr done";
            QString text=m_expression->result()->data().toString();
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
            Q_EMIT done();

            m_expression->deleteLater();
            m_expression=nullptr;
            break;
        }
        case Cantor::Expression::Error:
        {
            qWarning() << "syntax object error" << m_expression->result()->toHtml();
            Q_EMIT done();

            m_expression->deleteLater();
            m_expression=nullptr;
            break;
        }
        default:
            break;
    }
}
