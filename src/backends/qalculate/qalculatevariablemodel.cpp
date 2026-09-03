#include "qalculatevariablemodel.h"
#include "qalculatesession.h"

#include <libqalculate/Calculator.h>
#include <libqalculate/Function.h>

#include <QDebug>
#include <QTimer>

#include <KLocalizedString>

#include <algorithm>
#include <utility>

namespace
{

QStringList splitQalculateRow(const QString& row)
{
    QStringList values;
    qsizetype start = -1;
    int depth = 0;
    for (qsizetype index = 0; index < row.size(); ++index)
    {
        const QChar character = row.at(index);
        if (character == QLatin1Char('(') || character == QLatin1Char('['))
            ++depth;
        else if (character == QLatin1Char(')') || character == QLatin1Char(']'))
            --depth;

        const bool separator = depth == 0 && (character.isSpace() || character == QLatin1Char(','));
        if (separator)
        {
            if (start >= 0)
            {
                values.append(row.mid(start, index - start));
                start = -1;
            }
        }
        else if (start < 0)
            start = index;
    }
    if (start >= 0)
        values.append(row.mid(start));
    return values;
}

QList<QStringList> parseQalculateArray(const QString& input)
{
    const QString value = input.trimmed();
    if (!value.startsWith(QLatin1Char('[')) || !value.endsWith(QLatin1Char(']')))
        return {};

    const QString contents = value.mid(1, value.size() - 2).trimmed();
    QList<QStringList> rows;
    if (contents.contains(QLatin1Char(';')))
    {
        for (const QString& row : contents.split(QLatin1Char(';'), Qt::KeepEmptyParts))
            rows.append(splitQalculateRow(row.trimmed()));
        return rows;
    }

    const QStringList values = splitQalculateRow(contents);
    const bool nestedRows = !values.isEmpty() && std::all_of(values.cbegin(), values.cend(), [](const QString& row) {
        const QString trimmed = row.trimmed();
        return trimmed.startsWith(QLatin1Char('[')) && trimmed.endsWith(QLatin1Char(']'));
    });
    if (nestedRows)
    {
        for (const QString& row : values)
            rows.append(splitQalculateRow(row.trimmed().mid(1, row.trimmed().size() - 2)));
    }
    else
        rows.append(values);
    return rows;
}

Cantor::VariablePreviewData::Cell qalculateCell(const QString& value)
{
    Cantor::VariablePreviewData::Cell cell;
    cell.value = value;
    return cell;
}

Cantor::VariablePreviewData qalculatePreviewData(const QList<QStringList>& rows, qsizetype offset, qsizetype limit)
{
    Cantor::VariablePreviewData data;
    data.type = Cantor::VariablePreviewData::Type::Table;
    data.typeName = rows.size() > 1 ? QStringLiteral("matrix") : QStringLiteral("vector");
    data.offset = offset;

    if (rows.size() > 1)
    {
        qsizetype columns = 0;
        for (const auto& row : rows)
            columns = qMax(columns, row.size());
        data.totalRows = rows.size();
        data.dimensions = QStringLiteral("%1x%2").arg(data.totalRows).arg(columns);
        data.columnNames.append(QStringLiteral("@index"));
        for (qsizetype column = 0; column < columns; ++column)
            data.columnNames.append(QString::number(column + 1));
        for (qsizetype rowIndex = offset; rowIndex < qMin(offset + limit, data.totalRows); ++rowIndex)
        {
            QList<Cantor::VariablePreviewData::Cell> cells{qalculateCell(QString::number(rowIndex + 1))};
            for (qsizetype column = 0; column < columns; ++column)
                cells.append(qalculateCell(rows.at(rowIndex).value(column)));
            data.rows.append(std::move(cells));
        }
    }
    else
    {
        const QStringList values = rows.value(0);
        data.totalRows = values.size();
        data.dimensions = QString::number(data.totalRows);
        data.columnNames = {QStringLiteral("@index"), QStringLiteral("@value")};
        for (qsizetype index = offset; index < qMin(offset + limit, data.totalRows); ++index)
            data.rows.append({qalculateCell(QString::number(index + 1)), qalculateCell(values.at(index))});
    }
    data.hasMore = offset + limit < data.totalRows;
    return data;
}

}

using namespace Cantor;

QalculateVariableModel::QalculateVariableModel(QalculateSession* session) :
DefaultVariableModel(session), m_session(session)
{
}

QalculateVariableModel::~QalculateVariableModel()
{
}

void QalculateVariableModel::update()
{
    QList<Variable> newVars;
    QStringList newFuncs;

    const auto& sessionVars = m_session->getVariables();
    for (auto it = sessionVars.constBegin(); it != sessionVars.constEnd(); ++it)
        newVars.append(Variable(it.key(), it.value()));

    if (CALCULATOR) {
        for ( ExpressionItem* item : CALCULATOR->functions) 
            newFuncs << QLatin1String(item->name(true).c_str());
    }

    setVariables(newVars);
    setFunctions(newFuncs);

    setInitiallyPopulated();
}

Cantor::VariablePreviewData::Reference QalculateVariableModel::variablePreview(const QModelIndex& index) const
{
    auto reference = DefaultVariableModel::variablePreview(index);
    if (!index.isValid())
        return reference;

    if (!parseQalculateArray(variables().at(index.row()).value).isEmpty())
    {
        reference.type = Cantor::VariablePreviewData::Type::Table;
        reference.backendData = QByteArrayLiteral("{}");
    }
    return reference;
}

Cantor::VariablePreviewRequest* QalculateVariableModel::requestVariablePreview(const Cantor::VariablePreviewData::Reference& reference, qsizetype offset, qsizetype limit, QObject* parent)
{
    auto* request = new Cantor::VariablePreviewRequest(parent);
    const auto modelVariables = variables();
    const auto variable = std::find_if(modelVariables.cbegin(), modelVariables.cend(), [&reference](const auto& candidate) {
        return candidate.name == reference.variableName;
    });
    if (variable == modelVariables.cend())
    {
        QTimer::singleShot(0, request, [request]() {
            request->fail(i18n("The variable no longer exists."));
        });
        return request;
    }

    auto data = qalculatePreviewData(parseQalculateArray(variable->value), qMax<qsizetype>(0, offset), qMax<qsizetype>(1, limit));
    QTimer::singleShot(0, request, [request, data = std::move(data)]() mutable {
        request->complete(std::move(data));
    });
    return request;
}
