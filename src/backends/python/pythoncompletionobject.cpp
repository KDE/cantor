/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2013 Filipe Saraiva <filipe@kde.org>
*/

#include "pythoncompletionobject.h"

#include <QDebug>

#include "result.h"

#include "pythonsession.h"
#include "pythonkeywords.h"

using namespace Cantor;

PythonCompletionObject::PythonCompletionObject(const QString& command, int index, PythonSession* session) : Cantor::CompletionObject(session),
m_expression(nullptr)
{
    setLine(command, index);
}

PythonCompletionObject::~PythonCompletionObject()
{
    if (m_expression)
        m_expression->setFinishingBehavior(Expression::DeleteOnFinish);
}

void PythonCompletionObject::fetchCompletions()
{
    if (session()->status() != Session::Done)
    {
        QStringList allCompletions;

        allCompletions << PythonKeywords::instance()->variables();
        allCompletions << PythonKeywords::instance()->functions();
        allCompletions << PythonKeywords::instance()->keywords();

        setCompletions(allCompletions);

        Q_EMIT fetchingDone();
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
        connect(m_expression, &Cantor::Expression::statusChanged, this, &PythonCompletionObject::extractCompletions);
    }
}



void PythonCompletionObject::fetchIdentifierType()
{
    if (session()->status() != Cantor::Session::Done)
    {
        if (std::binary_search(PythonKeywords::instance()->functions().begin(),
                PythonKeywords::instance()->functions().end(), identifier()))
        Q_EMIT fetchingTypeDone(FunctionType);
        else if (std::binary_search(PythonKeywords::instance()->keywords().begin(),
                PythonKeywords::instance()->keywords().end(), identifier()))
        Q_EMIT fetchingTypeDone(KeywordType);
        else
        Q_EMIT fetchingTypeDone(VariableType);
    }
    else
    {
        if (m_expression)
            return;

        const QString& expr = QString::fromLatin1("callable(%1)").arg(identifier());
        m_expression = session()->evaluateExpression(expr, Cantor::Expression::FinishingBehavior::DoNotDelete, true);
        connect(m_expression, &Cantor::Expression::statusChanged, this, &PythonCompletionObject::extractIdentifierType);
    }
}

void PythonCompletionObject::extractCompletions(Cantor::Expression::Status status)
{
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
    Q_EMIT fetchingDone();
}

void PythonCompletionObject::extractIdentifierType(Cantor::Expression::Status status)
{
    switch(status)
    {
        case Cantor::Expression::Error:

            if (m_expression->errorMessage().contains(QLatin1String("SyntaxError: invalid syntax")))
                Q_EMIT fetchingTypeDone(KeywordType);
            else
            {
                qDebug() << "Error with PythonCompletionObject" << (m_expression->result() ? m_expression->result()->toHtml() : QLatin1String("extractIdentifierType"));
                Q_EMIT fetchingTypeDone(UnknownType);
            }
            break;

        case Cantor::Expression::Interrupted:
            qDebug() << "PythonCompletionObject was interrupted";
            Q_EMIT fetchingTypeDone(UnknownType);
            break;

        case Cantor::Expression::Done:
            if (m_expression->result())
            {
                if (m_expression->result()->data().toString() == QLatin1String("True"))
                    Q_EMIT fetchingTypeDone(FunctionType);
                else
                    Q_EMIT fetchingTypeDone(VariableType);
            }
            else
                Q_EMIT fetchingTypeDone(UnknownType);
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
