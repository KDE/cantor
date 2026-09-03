/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "maximavariablemodel.h"

#include "maximasession.h"
#include "maximaexpression.h"
#include "textresult.h"
#include "latexresult.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <KLocalizedString>

#include "settings.h"

#include <algorithm>
#include <utility>

//command used to inspect a maxima variable. %1 is the name of that variable
const QString MaximaVariableModel::inspectCommand=QLatin1String(":lisp($disp $%1)");
const QString MaximaVariableModel::variableInspectCommand=QLatin1String(":lisp(cantor-inspect $%1)");

namespace
{
struct MaximaPreviewValue
{
    enum class Kind
    {
        Scalar,
        List,
        Matrix
    };

    Kind kind{Kind::Scalar};
    QString value;
    QList<MaximaPreviewValue> items;
};

QStringList splitMaximaValues(const QString& text)
{
    QStringList values;
    qsizetype start = 0;
    int squareDepth = 0;
    int roundDepth = 0;
    bool quoted = false;
    bool escaped = false;
    for (qsizetype i = 0; i < text.size(); ++i)
    {
        const QChar character = text.at(i);
        if (quoted)
        {
            if (escaped)
                escaped = false;
            else if (character == QLatin1Char('\\'))
                escaped = true;
            else if (character == QLatin1Char('"'))
                quoted = false;
            continue;
        }

        if (character == QLatin1Char('"'))
            quoted = true;
        else if (character == QLatin1Char('['))
            ++squareDepth;
        else if (character == QLatin1Char(']'))
            --squareDepth;
        else if (character == QLatin1Char('('))
            ++roundDepth;
        else if (character == QLatin1Char(')'))
            --roundDepth;
        else if (character == QLatin1Char(',') && squareDepth == 0 && roundDepth == 0)
        {
            values.append(text.mid(start, i - start).trimmed());
            start = i + 1;
        }
    }
    if (start < text.size() || !text.trimmed().isEmpty())
        values.append(text.mid(start).trimmed());
    return values;
}

MaximaPreviewValue parseMaximaValue(const QString& input)
{
    const QString value = input.trimmed();
    MaximaPreviewValue parsed;
    parsed.value = value;

    if (value.startsWith(QLatin1String("matrix(")) && value.endsWith(QLatin1Char(')')))
    {
        parsed.kind = MaximaPreviewValue::Kind::Matrix;
        const QStringList rows = splitMaximaValues(value.mid(7, value.size() - 8));
        for (const QString& row : rows)
            parsed.items.append(parseMaximaValue(row));
    }
    else if (value.startsWith(QLatin1Char('[')) && value.endsWith(QLatin1Char(']')))
    {
        parsed.kind = MaximaPreviewValue::Kind::List;
        const QString contents = value.mid(1, value.size() - 2).trimmed();
        if (!contents.isEmpty())
            for (const QString& item : splitMaximaValues(contents))
                parsed.items.append(parseMaximaValue(item));
    }
    return parsed;
}

QString maximaPreviewTypeName(const MaximaPreviewValue& value)
{
    if (value.kind == MaximaPreviewValue::Kind::Matrix)
        return QStringLiteral("matrix");
    if (value.kind == MaximaPreviewValue::Kind::List)
        return QStringLiteral("list");
    return QStringLiteral("expression");
}

QString maximaPreviewText(QString value)
{
    value.replace(QLatin1Char('\n'), QLatin1Char(' '));
    constexpr qsizetype maximumLength = 200;
    if (value.size() > maximumLength)
        value = value.left(maximumLength - 3) + QStringLiteral("...");
    return value;
}

QByteArray maximaReferenceData(const QList<int>& path)
{
    QJsonArray jsonPath;
    for (const int index : path)
        jsonPath.append(index);
    QJsonObject object;
    object.insert(QLatin1String("path"), jsonPath);
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

Cantor::VariablePreviewData::Cell maximaCell(const QString& value, const QString& type = QString())
{
    Cantor::VariablePreviewData::Cell cell;
    cell.value = value;
    cell.type = type;
    return cell;
}

Cantor::VariablePreviewData maximaPreviewData(const MaximaPreviewValue& value, const QString& variableName, const QString& displayName, const QList<int>& path, qsizetype offset, qsizetype limit)
{
    using PreviewData = Cantor::VariablePreviewData;
    PreviewData data;
    data.type = PreviewData::Type::Table;
    data.typeName = maximaPreviewTypeName(value);
    data.offset = offset;

    if (value.kind == MaximaPreviewValue::Kind::Matrix)
    {
        qsizetype columnCount = 0;
        for (const auto& row : value.items)
            columnCount = qMax(columnCount, row.items.size());
        data.totalRows = value.items.size();
        data.dimensions = QStringLiteral("%1x%2").arg(data.totalRows).arg(columnCount);
        data.columnNames.append(QStringLiteral("@index"));
        for (qsizetype column = 0; column < columnCount; ++column)
            data.columnNames.append(QString::number(column + 1));

        for (qsizetype rowIndex = offset; rowIndex < qMin(offset + limit, data.totalRows); ++rowIndex)
        {
            QList<PreviewData::Cell> cells;
            cells.append(maximaCell(QString::number(rowIndex + 1), QStringLiteral("integer")));
            const auto& row = value.items.at(rowIndex);
            for (qsizetype column = 0; column < columnCount; ++column)
                cells.append(maximaCell(column < row.items.size() ? row.items.at(column).value : QString(), QStringLiteral("expression")));
            data.rows.append(std::move(cells));
        }
    }
    else
    {
        data.totalRows = value.items.size();
        data.dimensions = QString::number(data.totalRows);
        data.columnNames = {QStringLiteral("@index"), QStringLiteral("@type"), QStringLiteral("@value")};
        for (qsizetype index = offset; index < qMin(offset + limit, data.totalRows); ++index)
        {
            const auto& item = value.items.at(index);
            PreviewData::Cell previewCell;
            previewCell.type = maximaPreviewTypeName(item);
            previewCell.value = maximaPreviewText(item.value);
            if (item.kind != MaximaPreviewValue::Kind::Scalar)
            {
                QList<int> childPath = path;
                childPath.append(index);
                previewCell.reference.variableName = variableName;
                previewCell.reference.displayName = QStringLiteral("%1[%2]").arg(displayName).arg(index + 1);
                previewCell.reference.backendData = maximaReferenceData(childPath);
                previewCell.reference.type = PreviewData::Type::Table;
            }
            data.rows.append({maximaCell(QString::number(index + 1), QStringLiteral("integer")), maximaCell(previewCell.type), previewCell});
        }
    }

    data.hasMore = offset + limit < data.totalRows;
    return data;
}
}

MaximaVariableModel::MaximaVariableModel(MaximaSession* session) : Cantor::DefaultVariableModel(session)
{
}

void MaximaVariableModel::update()
{
    if (static_cast<MaximaSession*>(session())->mode() != MaximaSession::Maxima)
        return;

    if (!m_variableExpression)
    {
        qDebug()<<"checking for new variables";
        const QString& cmd1 = variableInspectCommand.arg(QLatin1String("values"));
        m_variableExpression = static_cast<MaximaExpression*>(session()->evaluateExpression(cmd1, Cantor::Expression::FinishingBehavior::DoNotDelete, true));
        connect(m_variableExpression, &Cantor::Expression::statusChanged, this, &MaximaVariableModel::parseNewVariables);
    }

    if (!m_functionExpression)
    {
        qDebug()<<"checking for new functions";
        const QString& cmd2 = inspectCommand.arg(QLatin1String("functions"));
        m_functionExpression = static_cast<MaximaExpression*>(session()->evaluateExpression(cmd2, Cantor::Expression::FinishingBehavior::DoNotDelete, true));
        connect(m_functionExpression, &Cantor::Expression::statusChanged, this, &MaximaVariableModel::parseNewFunctions);
    }
}

Cantor::VariablePreviewData::Reference MaximaVariableModel::variablePreview(const QModelIndex& index) const
{
    auto reference = DefaultVariableModel::variablePreview(index);
    if (!index.isValid())
        return reference;

    const auto modelVariables = variables();
    const MaximaPreviewValue value = parseMaximaValue(modelVariables.at(index.row()).value);
    if (value.kind != MaximaPreviewValue::Kind::Scalar)
    {
        reference.type = Cantor::VariablePreviewData::Type::Table;
        reference.backendData = maximaReferenceData({});
    }
    return reference;
}

Cantor::VariablePreviewRequest* MaximaVariableModel::requestVariablePreview(const Cantor::VariablePreviewData::Reference& reference, qsizetype offset, qsizetype limit, QObject* parent)
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

    MaximaPreviewValue value = parseMaximaValue(variable->value);
    QList<int> path;
    const QJsonDocument document = QJsonDocument::fromJson(reference.backendData);
    for (const auto& pathValue : document.object().value(QLatin1String("path")).toArray())
    {
        const int index = pathValue.toInt(-1);
        if (value.kind != MaximaPreviewValue::Kind::List || index < 0 || index >= value.items.size())
        {
            QTimer::singleShot(0, request, [request]() {
                request->fail(i18n("The variable or nested value no longer exists."));
            });
            return request;
        }
        path.append(index);
        value = value.items.at(index);
    }

    const auto data = maximaPreviewData(value, reference.variableName, reference.displayName, path, qMax<qsizetype>(0, offset), qMax<qsizetype>(1, limit));
    QTimer::singleShot(0, request, [request, data]() mutable {
        request->complete(std::move(data));
    });
    return request;
}

QList<Cantor::DefaultVariableModel::Variable> parse(MaximaExpression* expr)
{
    if(!expr
        || (expr->status()!=Cantor::Expression::Done && expr->errorMessage() != QLatin1String("$DONE"))
        || expr->results().isEmpty())
    {
        return QList<Cantor::DefaultVariableModel::Variable>();
    }

    //for parsing of names and values below (old code) we need to combine multiple results back to one string
    QString text;
    for (auto* result : expr->results())
    {
        if(result->type()==Cantor::TextResult::Type)
            text += static_cast<Cantor::TextResult*>(result)->plain();
        else if(expr->result()->type()==Cantor::LatexResult::Type)
            text += static_cast<Cantor::LatexResult*>(result)->plain();
    }

    const int nameIndex = text.indexOf(QLatin1Char(']'));
    QString namesString = text.left(nameIndex);
    //namesString.chop(1);
    namesString=namesString.mid(1);
    namesString=namesString.trimmed();

    qDebug()<<"variable names: "<<namesString;
    if(namesString.isEmpty())
        return QList<Cantor::DefaultVariableModel::Variable>();

    QStringList variableNames;
    QString valuesString;
    bool hasValues = false;
    QStringList variableValues;
    if ( namesString.contains(QLatin1Char(')')) )
    {
        //function definition(s): e.g
        //text = "[f1(x),f2(x,y),f3(x,y,z)]\n$DONE"
        //nameString = f1(x),f2(x,y),f3(x,y,z)
        //variableString = "\n$DONE"
        variableNames = namesString.split(QLatin1String("),"));
    }
    else
    {
        //variable definition(s): e.g.
        //text = "[a,b]\n1\n\"-cantor-value-separator-\"\n2\n\"-cantor-value-separator-\"\n($A $B)"
        //nameString = "[a,b]"
        //variableString = "\n1\n\"-cantor-value-separator-\"\n2\n\"-cantor-value-separator-\"\n($A $B)"
        variableNames = splitMaximaValues(namesString);
        if (MaximaSettings::self()->variableManagement())
        {
            valuesString = text.mid(nameIndex+1).trimmed();
            valuesString = valuesString.remove(QLatin1String("\n")); //lists with many elements have line breaks, remove them
            variableValues = valuesString.split(QLatin1String("\"-cantor-value-separator-\""));
            hasValues = !variableValues.isEmpty();
        }
    }

    qDebug()<<variableNames;
    qDebug()<<"string: "<<valuesString;
    qDebug()<<"values: "<<variableValues;
    qDebug()<<"has Values: "<<hasValues;

    QList<Cantor::DefaultVariableModel::Variable> variables;
    variables.reserve(variableNames.size());
    for(int i=0;i<variableNames.size();i++)
    {
        Cantor::DefaultVariableModel::Variable var;
        var.name = variableNames.at(i).trimmed();
        // Ignore abbreviations emitted by older Maxima initialization files.
        if (var.name.isEmpty() || var.name == QLatin1String("..."))
            continue;
        if(variableValues.size()>i)
        {
            var.value=variableValues.at(i).trimmed();
            var.value=var.value.remove(QLatin1String("\n")); //lists with many elements have line breaks, remove them

            // text output is quoted by Maxima, remove the quotes
            if (var.value.startsWith(QLatin1String("\"")))
            {
                var.value.remove(0, 1);
                var.value.chop(1);
                var.value.replace(QLatin1String("\\\""), QLatin1String("\""));
            }
        }
        else
            var.value=QLatin1String("unknown");

        variables<<var;
    }

    return variables;
}

void MaximaVariableModel::parseNewVariables(Cantor::Expression::Status status)
{
    if (status != Cantor::Expression::Done && status != Cantor::Expression::Error)
        return;

    qDebug()<<"parsing variables";

    QList<Variable> newVars=parse(m_variableExpression);
    setVariables(newVars);

    //the expression is not needed anymore
    m_variableExpression->deleteLater();
    m_variableExpression = nullptr;
}

void MaximaVariableModel::parseNewFunctions(Cantor::Expression::Status status)
{
    if (status != Cantor::Expression::Done && status != Cantor::Expression::Error)
        return;

    qDebug()<<"parsing functions";

    // List of variables?
    QList<Variable> newFuncs=parse(m_functionExpression);
    QStringList functions;
    for (Variable var : newFuncs)
        functions << var.name.left(var.name.indexOf(QLatin1Char('(')));
    qDebug() << functions;
    setFunctions(functions);

    //the expression is not needed anymore
    m_functionExpression->deleteLater();
    m_functionExpression = nullptr;
}

MaximaSession* MaximaVariableModel::maximaSession()
{
    return static_cast<MaximaSession*> (session());
}
