/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018 Nikita Sirgienko <warquark@gmail.com>
*/

#include "rvariablemodel.h"
#include "rsession.h"

#include <result.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

using namespace Cantor;

RVariableModel::RVariableModel(RSession* session) : DefaultVariableModel(session)
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

            const QStringList& names = output.section(unitSep, 0, 0).split(recordSep, Qt::SkipEmptyParts);
            const QStringList& values = output.section(unitSep, 1, 1).split(recordSep, Qt::KeepEmptyParts);
            QStringList funcs = output.section(unitSep, 2, 2).split(recordSep, Qt::SkipEmptyParts);
            const QStringList& constants = output.section(unitSep, 3, 3).split(recordSep, Qt::SkipEmptyParts);
            const QStringList& types = output.section(unitSep, 4, 4).split(recordSep, Qt::KeepEmptyParts);
            const QStringList& dimensions = output.section(unitSep, 5, 5).split(recordSep, Qt::KeepEmptyParts);

            QList<Variable> vars;
            if (!values.isEmpty()) // Variables management disabled
                for (int i = 0; i < names.size(); i++)
                {
                    if (i < values.size())
                        vars.append(Variable{names.at(i), values.at(i), 0, types.value(i), dimensions.value(i)});
                    else
                        vars.append(Variable{names.at(i), QString(), 0, types.value(i), dimensions.value(i)});
                }
            else
                for (int i = 0; i < names.size(); i++)
                    vars.append(Variable{names.at(i), QString()});
            setVariables(vars);

            // Remove primitive function "(" because it not function for user calling (i guess)
            // And the function with name like this make highlighting worse actually
            funcs.removeOne(QLatin1String("("));

            setFunctions(funcs);
            setConstants(constants);
            setInitiallyPopulated();
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

Cantor::VariablePreviewData::Reference RVariableModel::variablePreview(const QModelIndex& index) const
{
    auto reference = DefaultVariableModel::variablePreview(index);
    if (!index.isValid())
        return reference;

    const auto modelVariables = variables();
    const auto& variable = modelVariables.at(index.row());
    const QString& type = variable.type;
    const QString& dimensions = variable.dimension;
    if (type == QLatin1String("data.frame") || type == QLatin1String("matrix") || type == QLatin1String("list") || type == QLatin1String("named list"))
        reference.type = type == QLatin1String("named list") ? VariablePreviewData::Type::Dictionary : VariablePreviewData::Type::Table;
    else if (!dimensions.contains(QLatin1Char('x')) && dimensions.toLongLong() > 1)
        reference.type = VariablePreviewData::Type::Table;

    if (reference.isPreviewable())
    {
        QJsonObject object;
        object.insert(QLatin1String("name"), reference.variableName);
        object.insert(QLatin1String("displayName"), reference.displayName);
        object.insert(QLatin1String("path"), QJsonArray());
        reference.backendData = QJsonDocument(object).toJson(QJsonDocument::Compact);
    }
    return reference;
}

Cantor::VariablePreviewRequest* RVariableModel::requestVariablePreview(const Cantor::VariablePreviewData::Reference& reference, qsizetype offset, qsizetype limit, QObject* parent)
{
    const QString command = QStringLiteral("%variable preview %1 %2 %3")
                                .arg(QString::fromLatin1(reference.backendData.toBase64()))
                                .arg(qMax<qsizetype>(0, offset))
                                .arg(qMax<qsizetype>(1, limit));
    return requestVariablePreviewFromCommand(command, reference.variableName, parent);
}

void RVariableModel::setConstants(QStringList newConstants)
{
    QStringList addedConstants;
    QStringList removedConstants;

    //remove the old variables
    int i = 0;
    while (i < m_constants.size())
    {
        //check if this var is present in the new variables
        bool found = false;
        for (const QString& constant : newConstants)
            if(m_constants[i] == constant)
            {
                found=true;
                break;
            }

        if(!found)
        {
            removedConstants << m_constants[i];
            m_constants.removeAt(i);
        }
        else
            i++;
    }

    for (const QString& constant : newConstants)
    {
        if (!m_constants.contains(constant))
        {
            addedConstants << constant;
            m_constants.append(constant);
        }
    }

    Q_EMIT constantsAdded(addedConstants);
    Q_EMIT constantsRemoved(removedConstants);
}
