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

#include "rvariablemodel.h"
#include "rsession.h"

#include <result.h>

using namespace Cantor;

RVariableModel::RVariableModel(RSession* session):
    DefaultVariableModel(session),
    m_expression(nullptr)
{
}

RVariableModel::~RVariableModel()
{
    if (m_expression)
        m_expression->setFinishingBehavior(Expression::FinishingBehavior::DeleteOnFinish);
}

void RVariableModel::update()
{
    if (m_expression)
        return;

    m_expression = session()->evaluateExpression(QLatin1String("%model update"), Expression::FinishingBehavior::DoNotDelete, true);
    connect(m_expression, &Expression::statusChanged, this, &RVariableModel::parseResult);
}

void RVariableModel::parseResult(Cantor::Expression::Status status)
{
    switch(status)
    {
        case Expression::Status::Done:
        {
            if (!m_expression->result())
                break;

            const QChar recordSep(30);
            const QChar unitSep(31);

            const QString output = m_expression->result()->data().toString();

            const QStringList names = output.section(unitSep, 0, 0).split(recordSep, QString::SkipEmptyParts);
            const QStringList values = output.section(unitSep, 1, 1).split(recordSep, QString::SkipEmptyParts);
            const QStringList funcs = output.section(unitSep, 2, 2).split(recordSep, QString::SkipEmptyParts);

            QList<Variable> vars;
            if (!values.isEmpty()) // Variables management disabled
                for (int i = 0; i < names.size(); i++)
                    vars.append(Variable{names[i], values[i]});
            else
                for (int i = 0; i < names.size(); i++)
                    vars.append(Variable{names[i], QString()});

            setVariables(vars);

            setFunctions(funcs);
            break;
        }
        case Expression::Status::Error:
            qWarning() << "R code for update variable model finishs with error message: " << m_expression->errorMessage();
            break;

        case Expression::Status::Interrupted:
            break;

        default:
            return;
    }

    m_expression->deleteLater();
    m_expression = nullptr;
}
