/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018 Nikita Sirgienko <warquark@gmail.com>
*/

#include "pythonvariablemodel.h"
#include "pythonsession.h"
#include "textresult.h"

#include <QDebug>
#include <QDBusReply>
#include <QDBusInterface>
#include <QString>

#include "settings.h"

using namespace Cantor;

PythonVariableModel::PythonVariableModel(PythonSession* session):
    DefaultVariableModel(session)
{
}

PythonVariableModel::~PythonVariableModel()
{
    if (m_expression)
        m_expression->setFinishingBehavior(Cantor::Expression::FinishingBehavior::DeleteOnFinish);
}

void PythonVariableModel::update()
{
    if (m_expression)
        return;

    int variableManagement = PythonSettings::variableManagement();
    const QString command = QString::fromLatin1("%variables %1").arg(variableManagement);
    m_expression = session()->evaluateExpression(command, Cantor::Expression::FinishingBehavior::DoNotDelete, true);
    connect(m_expression, &Cantor::Expression::statusChanged, this, &PythonVariableModel::extractVariables);
}

void PythonVariableModel::extractVariables(Cantor::Expression::Status status)
{
    switch(status)
    {
        case Cantor::Expression::Done:
        {
            Cantor::Result* result = m_expression->result();
            if (result)
            {
                const QString data = result->data().toString();
                // In Cantor server response DC2(18) is delimiter between variables
                const QStringList& records = data.split(QChar(18), QString::SkipEmptyParts);

                QList<Variable> variables;
                for (const QString& record : records)
                {
                    // DC1(17) is delimiter between variable name and its value.
                    const QString& name = record.section(QChar(17), 0, 0);
                    const QString& value = record.section(QChar(17), 1, 1);
                    const QString& size = record.section(QChar(17), 2, 2);

                    variables << Variable(name, value, size.toULongLong());
                }

                setVariables(variables);
            }
            break;
        }
        case Cantor::Expression::Interrupted:
        case Cantor::Expression::Error:
        {
            qDebug() << "python variable model update finished with status" << (status == Cantor::Expression::Error? "Error" : "Interrupted");
            if (status == Cantor::Expression::Error)
                qDebug() << "error message: " << m_expression->errorMessage();
            break;
        }
        default:
            return;
    }

    m_expression->deleteLater();
    m_expression=nullptr;
}
