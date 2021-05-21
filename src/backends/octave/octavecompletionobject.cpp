/*
    SPDX-FileCopyrightText: 2010 Miha Čančula <miha.cancula@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "octavecompletionobject.h"
#include "octavekeywords.h"

#include "session.h"
#include "result.h"

#include <QDebug>

OctaveCompletionObject::OctaveCompletionObject(const QString& command, int index, Cantor::Session* parent):
    CompletionObject(parent),
    m_expression(nullptr)
{
    setLine(command, index);
}

OctaveCompletionObject::~OctaveCompletionObject()
{
    if (m_expression)
        m_expression->setFinishingBehavior(Cantor::Expression::FinishingBehavior::DeleteOnFinish);
}

void OctaveCompletionObject::fetchCompletions()
{
    if (session()->status() != Cantor::Session::Done)
    {
        QStringList allCompletions;

        allCompletions << OctaveKeywords::instance()->functions();
        allCompletions << OctaveKeywords::instance()->keywords();

        setCompletions(allCompletions);

        emit fetchingDone();
    }
    else
    {
        if (m_expression)
            return;
        qDebug() << "Fetching completions for" << command();
        QString expr = QString::fromLatin1("completion_matches('%1')").arg(command());
        m_expression = session()->evaluateExpression(expr,Cantor::Expression::FinishingBehavior::DoNotDelete, true);
        connect(m_expression, &Cantor::Expression::statusChanged, this, &OctaveCompletionObject::extractCompletions);
    }
}

void OctaveCompletionObject::extractCompletions(Cantor::Expression::Status status)
{
    switch(status)
    {
        case Cantor::Expression::Done:
        {
            Cantor::Result* result = m_expression->result();
            if (result)
            {
                QString res = result->data().toString();
                QStringList completions = res.split(QLatin1String("\n"), QString::SkipEmptyParts);
                qDebug() << "Adding" << completions.size() << "completions";
                setCompletions( completions );
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
    m_expression=nullptr;
    emit fetchingDone();
}

void OctaveCompletionObject::fetchIdentifierType()
{
    if (session()->status() != Cantor::Session::Done)
    {
        qDebug() << "Fetching type of " << identifier();
        if (OctaveKeywords::instance()->keywords().contains(identifier()))
            emit fetchingTypeDone(KeywordType);
        else if (OctaveKeywords::instance()->functions().contains(identifier()))
            emit fetchingTypeDone(FunctionType);
        else
            emit fetchingTypeDone(UnknownType);
    }
    else
    {
        if (m_expression)
            return;
        qDebug() << "Fetching type of " << identifier();
        QString expr = QString::fromLatin1("__cantor_tmp__ = [exist('%1'), iskeyword('%1')], clear __cantor_tmp__").arg(identifier());
        m_expression = session()->evaluateExpression(expr, Cantor::Expression::FinishingBehavior::DoNotDelete, true);
        connect(m_expression, &Cantor::Expression::statusChanged, this, &OctaveCompletionObject::extractIdentifierType);
    }
}

void OctaveCompletionObject::extractIdentifierType(Cantor::Expression::Status status)
{
    switch(status)
    {
        case Cantor::Expression::Error:
            qDebug() << "Error with OctaveCompletionObject" << m_expression->errorMessage();
            emit fetchingTypeDone(UnknownType);
            break;

        case Cantor::Expression::Interrupted:
            qDebug() << "OctaveCompletionObject was interrupted";
            emit fetchingTypeDone(UnknownType);
            break;

        case Cantor::Expression::Done:
            if (m_expression->result())
            {
                QString res = m_expression->result()->data().toString();
                // Remove '__cantor_tmp__ = \n' from result string
                // size("__cantor_tmp__ = \n") == 18
                res.remove(0,18);

                const QStringList& ints = res.split(QLatin1String(" "), QString::SkipEmptyParts);
                if (ints.size() != 2)
                    emit fetchingTypeDone(UnknownType);
                else if (ints[1].toInt() == 1)
                    emit fetchingTypeDone(KeywordType);
                else if (ints[0].toInt() == 1)
                    emit fetchingTypeDone(VariableType);
                else if (ints[0].toInt() == 5 || ints[0].toInt() == 103)
                    emit fetchingTypeDone(FunctionType);
                else
                    emit fetchingTypeDone(UnknownType);
            }
            break;

        default:
            return;
    }

    m_expression->deleteLater();
    m_expression = nullptr;
}
