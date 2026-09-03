/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018 Nikita Sirgienko <warquark@gmail.com>
    SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
*/

#include "pythonvariablemodel.h"
#include "pythonpreviewutils.h"
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
                const QStringList& records = data.split(QChar(18), Qt::SkipEmptyParts);

                QList<Variable> variables;
                for (const QString& record : records)
                {
                    // DC1 separates name, value, size, type and dimensions.
                    const auto& elements = record.split(QChar(17), Qt::KeepEmptyParts);
                    int count = elements.count();
                    if (count < 4)
                        continue;

                    const QString& name = elements.at(0);
                    const QString& value = elements.at(1);
                    const QString& size = elements.at(2);
                    const QString& type = elements.at(3);
                    const QString dimensions = elements.value(4);
                    variables << Variable(name, value, size.toULongLong(), type, dimensions);
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

Cantor::VariablePreviewData::Reference PythonVariableModel::variablePreview(const QModelIndex& index) const
{
    auto reference = DefaultVariableModel::variablePreview(index);
    if (!index.isValid())
        return reference;

    const QString typeName = index.siblingAtColumn(2).data().toString();
    QString dimensions;
    for (const auto& variable : variables())
        if (variable.name == reference.variableName)
        {
            dimensions = variable.dimension;
            break;
        }
    reference.type = pythonPreviewType(typeName, dimensions);
    if (reference.isPreviewable())
        reference.backendData = pythonPreviewReference(reference.variableName);
    return reference;
}

Cantor::VariablePreviewRequest* PythonVariableModel::requestVariablePreview(const Cantor::VariablePreviewData::Reference& reference, qsizetype offset, qsizetype limit, QObject* parent)
{
    if (!reference.isPreviewable() || reference.backendData.isEmpty())
        return DefaultVariableModel::requestVariablePreview(reference, offset, limit, parent);

    const QString command = pythonPreviewCommand(reference, offset, limit);
    return requestVariablePreviewFromCommand(command, reference.variableName, parent);
}
