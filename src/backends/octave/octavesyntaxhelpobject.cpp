/*
    Copyright (C) 2010 Miha Čančula <miha.cancula@gmail.com>

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
*/

#include "octavesyntaxhelpobject.h"
#include "session.h"
#include "result.h"

#include <QDebug>

OctaveSyntaxHelpObject::OctaveSyntaxHelpObject(const QString& command, Cantor::Session* session): SyntaxHelpObject(command, session),
m_expression(nullptr)
{

}

void OctaveSyntaxHelpObject::fetchInformation()
{
    if (session()->status() != Cantor::Session::Disable)
    {
        qDebug() << "Fetching syntax help for" << command();
        QString expr = QString::fromLatin1("help('%1')").arg(command());
        m_expression = session()->evaluateExpression(expr, Cantor::Expression::FinishingBehavior::DoNotDelete, true);
        connect(m_expression, &Cantor::Expression::statusChanged, this, &OctaveSyntaxHelpObject::fetchingDone);
    }
    else
        emit done();
}

void OctaveSyntaxHelpObject::fetchingDone(Cantor::Expression::Status status)
{
    switch(status)
    {
        case Cantor::Expression::Done:
        {
            Cantor::Result* result = m_expression->result();
            if (result)
            {
                QString res = result->toHtml();
                res.remove(QLatin1String("<br/>"));
                res.remove(0, res.indexOf(QLatin1String("--")));
                setHtml(QLatin1Char(' ') + res.trimmed());
            }
            break;
        }

        case Cantor::Expression::Interrupted:
        case Cantor::Expression::Error:
        {
            qDebug() << "fetching expression finished with status" << (status == Cantor::Expression::Error? "Error" : "Interrupted");
            break;
        }

        default:
            return;
    }
    m_expression->deleteLater();
    m_expression = nullptr;
    emit done();
}
