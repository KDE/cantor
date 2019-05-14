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
    Copyright (C) 2018 Nikita Sirgienko <warquark@gmail.com>
*/

#include "pythonvariablemodel.h"
#include "pythonsession.h"
#include "textresult.h"

#include <QDebug>
#include <QDBusReply>
#include <QDBusInterface>
#include <QString>

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

    int variableManagement = static_cast<PythonSession*>(session())->variableManagement();
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

                    variables << Variable{name, value};
                }

                setVariables(variables);
            }
            break;
        }
        case Cantor::Expression::Interrupted:
        case Cantor::Expression::Error:
        {
            qDebug() << "python variable model update finished with status" << (status == Cantor::Expression::Error? "Error" : "Interrupted");
            break;
        }
        default:
            return;
    }

    m_expression->deleteLater();
    m_expression=nullptr;
}
