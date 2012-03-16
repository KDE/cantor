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

#include "octavecompletionobject.h"

#include "session.h"
#include "expression.h"
#include "result.h"

#include <KDebug>

OctaveCompletionObject::OctaveCompletionObject(const QString& command, int index, Cantor::Session* parent): CompletionObject(parent)
{
    setLine(command, index);
}

OctaveCompletionObject::~OctaveCompletionObject()
{

}

void OctaveCompletionObject::fetchCompletions()
{
    kDebug() << "Fetching completions for" << command();
    QString expr = QString("completion_matches(\"%1\")").arg(command());
    m_expression = session()->evaluateExpression(expr);
    connect (m_expression, SIGNAL(statusChanged(Cantor::Expression::Status)), SLOT(fetchingDone()));
}

void OctaveCompletionObject::fetchingDone()
{
    if (!m_expression || m_expression->status() != Cantor::Expression::Done)
    {
        return;
    }
    Cantor::Result* result = m_expression->result();
    if (result)
    {
        QString res = result->toHtml();
        QStringList completions = res.split("<br/>\n", QString::SkipEmptyParts);
        kDebug() << "Adding" << completions.size() << "completions";
        setCompletions( completions );
    }
    m_expression->deleteLater();
    emit done();
}
