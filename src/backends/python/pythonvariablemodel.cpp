/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018 Nikita Sirgienko <warquark@gmail.com>
    SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
*/

#include "pythonvariablemodel.h"
#include "pythonsession.h"
#include "result.h"

#include <QDebug>
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
            auto* result = m_expression->result();
            if (result)
            {
                const QString data = result->data().toString();
                // In Cantor server response DC2(18) is delimiter between variables
                const QStringList& records = data.split(QChar(18), QString::SkipEmptyParts);

                QList<Variable> variables;
                for (const QString& record : records)
                {
                    // every variable data has 4 parts/elements separated by DC1(17) - the name of the variable, its size, type and the actual value
                    const auto& elements = record.split(QChar(17), QString::SkipEmptyParts);
                    int count = elements.count();
                    if (count < 4)
                        continue;

                    const QString& name = elements.at(0);
                    const QString& value = elements.at(1);
                    const QString& size = elements.at(2);
                    const QString& type = elements.at(3);
                    variables << Variable(name, value, size.toULongLong(), type);
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
    m_expression = nullptr;
}
