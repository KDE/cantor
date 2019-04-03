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
    Copyright (C) 2013 Filipe Saraiva <filipe@kde.org>
 */

#include "pythoncompletionobject.h"

#include <QDebug>

#include "result.h"

#include "pythonsession.h"
#include "pythonkeywords.h"

PythonCompletionObject::PythonCompletionObject(const QString& command, int index, PythonSession* session) : Cantor::CompletionObject(session),
m_expression(nullptr)
{
    setLine(command, index);
}

void PythonCompletionObject::fetchCompletions()
{
    if (session()->status() == Cantor::Session::Disable)
    {
        QStringList allCompletions;

        allCompletions << PythonKeywords::instance()->variables();
        allCompletions << PythonKeywords::instance()->functions();
        allCompletions << PythonKeywords::instance()->keywords();

        setCompletions(allCompletions);

        emit fetchingDone();
    }
    else
    {
        if (m_expression)
            return;

        qDebug() << "run fetchCompletions";
        const QString& expr = QString::fromLatin1(
            "from __main__ import __dict__;"
            "import rlcompleter;"
            "print('|'.join(rlcompleter.Completer(__dict__).global_matches('%1')+rlcompleter.Completer(__dict__).attr_matches('%1')))"
        ).arg(command());
        m_expression = session()->evaluateExpression(expr, Cantor::Expression::FinishingBehavior::DoNotDelete, true);
        // TODO: Python exec the expression before connect, so manualy run handler. Uncomment the connection after removing DBus
        // connect(m_expression, &Cantor::Expression::statusChanged, this, &PythonCompletionObject::extractCompletions);
        extractCompletions(m_expression->status());
    }
}



void PythonCompletionObject::fetchIdentifierType()
{
    if (session()->status() == Cantor::Session::Disable)
    {
        if (qBinaryFind(PythonKeywords::instance()->functions().begin(),
                PythonKeywords::instance()->functions().end(), identifier())
        != PythonKeywords::instance()->functions().end())
        emit fetchingTypeDone(FunctionType);
        else if (qBinaryFind(PythonKeywords::instance()->keywords().begin(),
                PythonKeywords::instance()->keywords().end(), identifier())
        != PythonKeywords::instance()->keywords().end())
        emit fetchingTypeDone(KeywordType);
        else
        emit fetchingTypeDone(VariableType);
    }
    else
    {
        if (m_expression)
            return;

        const QString& expr = QString::fromLatin1("callable(%1)").arg(identifier());
        m_expression = session()->evaluateExpression(expr, Cantor::Expression::FinishingBehavior::DoNotDelete, true);
        // TODO: Python exec the expression before connect, so manualy run handler. Uncomment the connection after removing DBus
        // connect(m_expression, &Cantor::Expression::statusChanged, this, &PythonCompletionObject::extractIdentifierType);
        extractIdentifierType(m_expression->status());
    }
}

void PythonCompletionObject::extractCompletions(Cantor::Expression::Status status)
{
    if (!m_expression)
        return;

    switch(status)
    {
        case Cantor::Expression::Error:
            qDebug() << "Error with PythonCompletionObject" << (m_expression->result() ? m_expression->result()->toHtml() : QLatin1String("extractCompletions"));
            break;

        case Cantor::Expression::Interrupted:
            qDebug() << "PythonCompletionObject was interrupted";
            break;

        case Cantor::Expression::Done:
            if (m_expression->result())
                setCompletions(m_expression->result()->data().toString().remove(QLatin1Char('(')).split(QLatin1Char('|')));
            break;
        default:
            return;
    }
    m_expression->deleteLater();
    m_expression = nullptr;
    emit fetchingDone();
}

void PythonCompletionObject::extractIdentifierType(Cantor::Expression::Status status)
{
    if (!m_expression)
            return;
    switch(status)
    {
        case Cantor::Expression::Error:

            if (m_expression->errorMessage().contains(QLatin1String("SyntaxError: invalid syntax")))
                emit fetchingTypeDone(KeywordType);
            else
                qDebug() << "Error with PythonCompletionObject" << (m_expression->result() ? m_expression->result()->toHtml() : QLatin1String("extractIdentifierType"));
            break;

        case Cantor::Expression::Interrupted:
            qDebug() << "PythonCompletionObject was interrupted";
            break;

        case Cantor::Expression::Done:
            if (m_expression->result())
            {
                if (m_expression->result()->data().toString() == QLatin1String("True"))
                    emit fetchingTypeDone(FunctionType);
                else
                    emit fetchingTypeDone(VariableType);
            }
            else
                emit fetchingTypeDone(UnknownType);
            break;
        default:
            return;
    }
    m_expression->deleteLater();
    m_expression = nullptr;
}

bool PythonCompletionObject::mayIdentifierContain(QChar c) const
{
    return c.isLetter() || c.isDigit() || c == QLatin1Char('_') || c == QLatin1Char('%') || c == QLatin1Char('$') || c == QLatin1Char('.');
}

bool PythonCompletionObject::mayIdentifierBeginWith(QChar c) const
{
    return c.isLetter() || c == QLatin1Char('_') || c == QLatin1Char('%') || c == QLatin1Char('$');
}
