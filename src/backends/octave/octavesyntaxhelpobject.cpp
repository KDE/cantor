/*
    SPDX-FileCopyrightText: 2010 Miha Čančula <miha.cancula@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
        Q_EMIT done();
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
    Q_EMIT done();
}
