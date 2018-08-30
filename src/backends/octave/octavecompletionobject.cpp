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

#include <QDebug>

OctaveCompletionObject::OctaveCompletionObject(const QString& command, int index, Cantor::Session* parent): CompletionObject(parent)
{
    setLine(command, index);
    m_expression = nullptr;
}

void OctaveCompletionObject::fetchCompletions()
{
    if (m_expression)
	return;
    qDebug() << "Fetching completions for" << command();
    QString expr = QString::fromLatin1("completion_matches('%1')").arg(command());
    m_expression = session()->evaluateExpression(expr);
    connect(m_expression, &Cantor::Expression::statusChanged, this, &OctaveCompletionObject::extractCompletions);
}

void OctaveCompletionObject::extractCompletions()
{
    if (!m_expression)
	return;
    if (m_expression->status() != Cantor::Expression::Done)
    {
	m_expression->deleteLater();
	m_expression = nullptr;
        return;
    }
    Cantor::Result* result = m_expression->result();
    if (result)
    {
        QString res = result->toHtml();
        QStringList completions = res.split(QLatin1String("<br/>\n"), QString::SkipEmptyParts);
        qDebug() << "Adding" << completions.size() << "completions";
        setCompletions( completions );
    }
    m_expression->deleteLater();
    m_expression = nullptr;
    emit fetchingDone();
}

void OctaveCompletionObject::fetchIdentifierType()
{
    if (m_expression)
	return;
    qDebug() << "Fetching type of " << identifier();
    // The output should look like
    // sin is a built-in function
    // __cantor_tmp2__ = 5
    QString expr = QString::fromLatin1("ans = type('%1');ans").arg(identifier());
    m_expression = session()->evaluateExpression(expr);
    connect(m_expression, &Cantor::Expression::statusChanged, this, &OctaveCompletionObject::extractIdentifierType);
}

void OctaveCompletionObject::extractIdentifierType()
{
    qDebug() << "type fetching done";
    if (!m_expression)
	return;
    if (m_expression->status() != Cantor::Expression::Done)
    {
	m_expression->deleteLater();
	m_expression = nullptr;
        return;
    }
    Cantor::Result* result = m_expression->result();
    m_expression->deleteLater();
    m_expression = nullptr;
    if (!result)
	return;

    QString res = result->toHtml();
    int endOfLine1 = res.indexOf(QLatin1String("<br/>"));
    int endOfLine2 = res.indexOf(QLatin1String("<br/>"), endOfLine1+1);
    QString line1 = res.left(endOfLine1);
    QString line2 = res.mid(endOfLine1, endOfLine2-endOfLine1);
    // for functions defined on the command line type says "undefined",
    // but sets ans to 103
    if (line1.endsWith(QLatin1String("function")) || line1.contains(QLatin1String("user-defined function"))
	|| line2.endsWith(QLatin1String("103")))
	emit fetchingTypeDone(FunctionType);
    else if (res.endsWith(QLatin1String("variable")))
	emit fetchingTypeDone(VariableType);
    else if (res.endsWith(QLatin1String("keyword")))
	emit fetchingTypeDone(KeywordType);
    else
	emit fetchingTypeDone(UnknownType);
}
