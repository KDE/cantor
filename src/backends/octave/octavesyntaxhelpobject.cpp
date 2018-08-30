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

OctaveSyntaxHelpObject::OctaveSyntaxHelpObject(const QString& command, Cantor::Session* session): SyntaxHelpObject(command, session)
{

}

void OctaveSyntaxHelpObject::fetchInformation()
{
    qDebug() << "Fetching syntax help for" << command();
    QString expr = QString::fromLatin1("help('%1')").arg(command());
    m_expression = session()->evaluateExpression(expr);
    connect(m_expression, &Cantor::Expression::statusChanged, this, &OctaveSyntaxHelpObject::fetchingDone);
}

void OctaveSyntaxHelpObject::fetchingDone()
{
    if (!m_expression || m_expression->status() != Cantor::Expression::Done)
    {
        return;
    }
    Cantor::Result* result = m_expression->result();
    if (result)
    {
      QString res = result->toHtml();
      res.remove(QLatin1String("<br/>"));
      res.remove(0, res.indexOf(QLatin1String("--")));
      setHtml(QLatin1Char(' ') + res.trimmed());
    }
    m_expression->deleteLater();
    emit done();
}
