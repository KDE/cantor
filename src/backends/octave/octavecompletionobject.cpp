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
    m_expression = 0;
}

OctaveCompletionObject::~OctaveCompletionObject()
{

}

void OctaveCompletionObject::fetchCompletions()
{
    if (m_expression)
	return;
    kDebug() << "Fetching completions for" << command();
    QString expr = QString("completion_matches(\"%1\")").arg(command());
    m_expression = session()->evaluateExpression(expr);
    connect (m_expression, SIGNAL(statusChanged(Cantor::Expression::Status)), SLOT(extractCompletions()));
}

void OctaveCompletionObject::extractCompletions()
{
    if (!m_expression)
	return;
    if (m_expression->status() != Cantor::Expression::Done)
    {
	m_expression->deleteLater();
	m_expression = 0;
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
    m_expression = 0;
    emit fetchingDone();
}

void OctaveCompletionObject::fetchIdentifierType()
{
    if (m_expression)
	return;
    kDebug() << "Fetching type of " << identifier();
    // The ouput should look like
    // sin is a built-in function
    // __cantor_tmp2__ = 5
    QString expr = QString("__cantor_internal1__ = ans; type(\"%1\"); __cantor_internal2__ = ans; ans = __cantor_internal1__; __cantor_internal2__").arg(identifier());
    m_expression = session()->evaluateExpression(expr);
    connect (m_expression, SIGNAL(statusChanged(Cantor::Expression::Status)), SLOT(extractIdentifierType()));
}

void OctaveCompletionObject::extractIdentifierType()
{
    kDebug() << "type fetching done";
    if (!m_expression)
	return;
    if (m_expression->status() != Cantor::Expression::Done)
    {
	m_expression->deleteLater();
	m_expression = 0;
        return;
    }
    Cantor::Result* result = m_expression->result();
    m_expression->deleteLater();
    m_expression = 0;
    if (!result)
	return;

    QString res = result->toHtml();
    int endOfLine1 = res.indexOf("<br/>");
    int endOfLine2 = res.indexOf("<br/>", endOfLine1+1);
    QString line1 = res.left(endOfLine1);
    QString line2 = res.mid(endOfLine1, endOfLine2-endOfLine1);
    // for functions defined on the command line type says "undefined",
    // but sets ans to 103
    if (line1.endsWith("function") || line1.contains("user-defined function") 
	|| line2.endsWith("103"))
	emit fetchingTypeDone(FunctionType);
    else if (res.endsWith("variable"))
	emit fetchingTypeDone(VariableType);
    else if (res.endsWith("keyword"))
	emit fetchingTypeDone(KeywordType);
    else
	emit fetchingTypeDone(UnknownType);
}
