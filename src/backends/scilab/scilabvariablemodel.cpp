#include "scilabvariablemodel.h"

#include "result.h"

#include <KLocalizedString>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSet>
#include <QTimer>

#include <algorithm>
#include <utility>

namespace
{

struct ScilabValue
{
    enum class Kind
    {
        Scalar,
        List,
        Matrix,
        Dictionary
    };

    Kind kind{Kind::Scalar};
    QString text;
    QString type;
    QStringList keys;
    QList<ScilabValue> items;
};

QStringList splitScilabValues(const QString& text, QChar separator)
{
    QStringList values;
    qsizetype start = 0;
    int squareDepth = 0;
    int roundDepth = 0;
    bool quoted = false;
    for (qsizetype index = 0; index < text.size(); ++index)
    {
        const QChar character = text.at(index);
        if (character == QLatin1Char('"'))
        {
            if (quoted && index + 1 < text.size() && text.at(index + 1) == QLatin1Char('"'))
            {
                ++index;
                continue;
            }
            quoted = !quoted;
            continue;
        }
        if (quoted)
            continue;

        if (character == QLatin1Char('['))
            ++squareDepth;
        else if (character == QLatin1Char(']'))
            --squareDepth;
        else if (character == QLatin1Char('('))
            ++roundDepth;
        else if (character == QLatin1Char(')'))
            --roundDepth;
        else if (character == separator && squareDepth == 0 && roundDepth == 0)
        {
            values.append(text.mid(start, index - start).trimmed());
            start = index + 1;
        }
    }
    if (start < text.size() || !text.trimmed().isEmpty())
        values.append(text.mid(start).trimmed());
    return values;
}

QString scilabScalarType(const QString& value)
{
    if (value.startsWith(QLatin1Char('"')) && value.endsWith(QLatin1Char('"')))
        return QStringLiteral("string");
    if (value == QLatin1String("%t") || value == QLatin1String("%f"))
        return QStringLiteral("boolean");
    static const QRegularExpression numberPattern(QStringLiteral(R"(^[+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eEdD][+-]?\d+)?$)"));
    if (numberPattern.match(value).hasMatch())
        return QStringLiteral("constant");
    return QStringLiteral("expression");
}

QString scilabScalarText(QString value)
{
    if (value.size() >= 2 && value.startsWith(QLatin1Char('"')) && value.endsWith(QLatin1Char('"')))
    {
        value = value.mid(1, value.size() - 2);
        value.replace(QStringLiteral("\"\""), QStringLiteral("\""));
    }
    return value;
}

ScilabValue parseScilabValue(const QString& input)
{
    const QString value = input.trimmed();
    ScilabValue parsed;
    parsed.text = scilabScalarText(value);
    parsed.type = scilabScalarType(value);

    const auto parseSequence = [&parsed](const QString& contents) {
        parsed.kind = ScilabValue::Kind::List;
        parsed.type = QStringLiteral("list");
        if (!contents.trimmed().isEmpty())
            for (const QString& item : splitScilabValues(contents, QLatin1Char(',')))
                parsed.items.append(parseScilabValue(item));
    };

    if (value.startsWith(QLatin1String("list(")) && value.endsWith(QLatin1Char(')')))
        parseSequence(value.mid(5, value.size() - 6));
    else if ((value.startsWith(QLatin1String("tlist(")) || value.startsWith(QLatin1String("mlist("))) && value.endsWith(QLatin1Char(')')))
        parseSequence(value.mid(6, value.size() - 7));
    else if (value.startsWith(QLatin1String("struct(")) && value.endsWith(QLatin1Char(')')))
    {
        const QStringList fields = splitScilabValues(value.mid(7, value.size() - 8), QLatin1Char(','));
        if (fields.size() % 2 == 0)
        {
            parsed.kind = ScilabValue::Kind::Dictionary;
            parsed.type = QStringLiteral("struct");
            for (qsizetype index = 0; index < fields.size(); index += 2)
            {
                parsed.keys.append(scilabScalarText(fields.at(index)));
                parsed.items.append(parseScilabValue(fields.at(index + 1)));
            }
        }
    }
    else if (value.startsWith(QLatin1Char('[')) && value.endsWith(QLatin1Char(']')))
    {
        const QString contents = value.mid(1, value.size() - 2).trimmed();
        const QStringList rows = splitScilabValues(contents, QLatin1Char(';'));
        if (rows.size() > 1)
        {
            parsed.kind = ScilabValue::Kind::Matrix;
            parsed.type = QStringLiteral("matrix");
            for (const QString& rowText : rows)
            {
                ScilabValue row;
                row.kind = ScilabValue::Kind::List;
                for (const QString& item : splitScilabValues(rowText, QLatin1Char(',')))
                    row.items.append(parseScilabValue(item));
                parsed.items.append(std::move(row));
            }
        }
        else
            parseSequence(contents);
    }

    return parsed;
}

QString decodeScilabText(const QString& encoded)
{
    QByteArray bytes;
    const QStringList codes = encoded.split(QLatin1Char(','), Qt::SkipEmptyParts);
    bytes.reserve(codes.size());
    for (const QString& code : codes)
    {
        bool ok = false;
        const uint value = code.toUInt(&ok);
        if (!ok || value > 255)
            return {};
        bytes.append(static_cast<char>(value));
    }
    return QString::fromUtf8(bytes);
}

bool isScilabSystemVariable(const QString& name, const QString& type)
{
    static const QSet<QString> systemVariables{QStringLiteral("PWD"), QStringLiteral("SCI"), QStringLiteral("SCIHOME"), QStringLiteral("TMPDIR"), QStringLiteral("enull"), QStringLiteral("evoid"), QStringLiteral("home"), QStringLiteral("jnull"), QStringLiteral("jvoid"), QStringLiteral("percentchars")};
    return name.startsWith(QLatin1Char('%')) || name.startsWith(QLatin1String("__cantor_")) || systemVariables.contains(name) || type == QLatin1String("library");
}

QString scilabTypeName(const ScilabValue& value)
{
    switch (value.kind)
    {
        case ScilabValue::Kind::List:
            return QStringLiteral("list");
        case ScilabValue::Kind::Matrix:
            return QStringLiteral("matrix");
        case ScilabValue::Kind::Dictionary:
            return QStringLiteral("struct");
        case ScilabValue::Kind::Scalar:
            return value.type;
    }
    return {};
}

QByteArray scilabReferenceData(const QList<int>& path)
{
    QJsonArray jsonPath;
    for (const int index : path)
        jsonPath.append(index);
    QJsonObject object;
    object.insert(QLatin1String("path"), jsonPath);
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

Cantor::VariablePreviewData::Cell scilabCell(const QString& value, const QString& type = QString())
{
    Cantor::VariablePreviewData::Cell cell;
    cell.value = value;
    cell.type = type;
    return cell;
}

Cantor::VariablePreviewData scilabPreviewData(const ScilabValue& value, const QString& variableName, const QString& displayName, const QList<int>& path, qsizetype offset, qsizetype limit)
{
    using PreviewData = Cantor::VariablePreviewData;
    PreviewData data;
    data.type = value.kind == ScilabValue::Kind::Dictionary ? PreviewData::Type::Dictionary : PreviewData::Type::Table;
    data.typeName = scilabTypeName(value);
    data.offset = offset;

    if (value.kind == ScilabValue::Kind::Matrix)
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
            QList<PreviewData::Cell> row{scilabCell(QString::number(rowIndex + 1), QStringLiteral("constant"))};
            for (const auto& item : value.items.at(rowIndex).items)
                row.append(scilabCell(item.text, item.type));
            while (row.size() < columnCount + 1)
                row.append(scilabCell(QString()));
            data.rows.append(std::move(row));
        }
    }
    else
    {
        data.totalRows = value.items.size();
        data.dimensions = QString::number(data.totalRows);
        data.columnNames = value.kind == ScilabValue::Kind::Dictionary ? QStringList{QStringLiteral("@key"), QStringLiteral("@type"), QStringLiteral("@value")} : QStringList{QStringLiteral("@index"), QStringLiteral("@type"), QStringLiteral("@value")};
        for (qsizetype index = offset; index < qMin(offset + limit, data.totalRows); ++index)
        {
            const auto& item = value.items.at(index);
            PreviewData::Cell valueCell = scilabCell(item.kind == ScilabValue::Kind::Scalar ? item.text : QStringLiteral("<%1>").arg(scilabTypeName(item)), scilabTypeName(item));
            if (item.kind != ScilabValue::Kind::Scalar)
            {
                QList<int> childPath = path;
                childPath.append(index);
                valueCell.reference.variableName = variableName;
                valueCell.reference.displayName = value.kind == ScilabValue::Kind::Dictionary ? QStringLiteral("%1[%2]").arg(displayName, value.keys.value(index)) : QStringLiteral("%1[%2]").arg(displayName).arg(index + 1);
                valueCell.reference.backendData = scilabReferenceData(childPath);
                valueCell.reference.type = item.kind == ScilabValue::Kind::Dictionary ? PreviewData::Type::Dictionary : PreviewData::Type::Table;
            }
            const QString label = value.kind == ScilabValue::Kind::Dictionary ? value.keys.value(index) : QString::number(index + 1);
            data.rows.append({scilabCell(label), scilabCell(scilabTypeName(item)), std::move(valueCell)});
        }
    }

    data.hasMore = offset + limit < data.totalRows;
    return data;
}

}

ScilabVariableModel::ScilabVariableModel(Cantor::Session* session)
    : Cantor::DefaultVariableModel(session)
{
}

void ScilabVariableModel::update()
{
    if (m_expression)
    {
        m_updatePending = true;
        return;
    }

    m_updatePending = false;

    static const QString command = QStringLiteral(R"SCILAB(
function __cantor_encoded__ = __cantor_encode__(__cantor_text__)
    __cantor_codes__ = ascii(__cantor_text__);
    if isempty(__cantor_codes__) then
        __cantor_encoded__ = "";
    else
        __cantor_encoded__ = strcat(string(__cantor_codes__), ",");
    end
endfunction

__cantor_names__ = who("local");
for __cantor_index__ = 1:size(__cantor_names__, "*")
    __cantor_name__ = __cantor_names__(__cantor_index__);
    try
        __cantor_system__ = or(__cantor_name__ == ["PWD", "SCI", "SCIHOME", "TMPDIR", "enull", "evoid", "home", "jnull", "jvoid", "percentchars"]);
        __cantor_prefix__ = strindex(__cantor_name__, "__cantor_");
        __cantor_internal__ = %f;
        if ~isempty(__cantor_prefix__) then
            __cantor_internal__ = __cantor_prefix__(1) == 1;
        end
        if part(__cantor_name__, 1) <> "%" & ~__cantor_system__ & ~__cantor_internal__ then
            __cantor_type__ = typeof(evstr(__cantor_name__));
        else
            __cantor_type__ = "library";
        end
        if __cantor_type__ <> "library" then
            if __cantor_type__ == "function" | __cantor_type__ == "fptr" then
                __cantor_value__ = "";
                __cantor_dimensions__ = "";
            else
                __cantor_object__ = evstr(__cantor_name__);
                try
                    __cantor_size__ = size(__cantor_object__);
                    __cantor_dimensions__ = strcat(string(__cantor_size__), "x");
                catch
                    __cantor_size__ = [];
                    __cantor_dimensions__ = "";
                end
                if size(__cantor_size__, "*") > 2 then
                    __cantor_value__ = "<multidimensional>";
                else
                    try
                        __cantor_value__ = sci2exp(__cantor_object__);
                    catch
                        __cantor_value__ = "<unprintable>";
                    end
                end
            end
            mprintf("__CANTOR_SCILAB_VARIABLE__%s|%s|%s|%s\n", __cantor_encode__(__cantor_name__), __cantor_encode__(__cantor_value__), __cantor_encode__(__cantor_type__), __cantor_encode__(__cantor_dimensions__));
        end
    catch
    end
end
clear __cantor_names__ __cantor_index__ __cantor_name__ __cantor_object__ __cantor_type__ __cantor_value__ __cantor_size__ __cantor_dimensions__ __cantor_system__ __cantor_prefix__ __cantor_internal__ __cantor_encode__;
)SCILAB");

    m_expression = session()->evaluateExpression(command, Cantor::Expression::FinishingBehavior::DoNotDelete, true);
    connect(m_expression, &Cantor::Expression::statusChanged, this, &ScilabVariableModel::parseVariables);
}

void ScilabVariableModel::parseVariables(Cantor::Expression::Status status)
{
    if (status == Cantor::Expression::Status::Computing || status == Cantor::Expression::Status::Queued)
        return;

    if (status == Cantor::Expression::Status::Done && m_expression && m_expression->result())
    {
        static const QLatin1String marker("__CANTOR_SCILAB_VARIABLE__");
        QList<Variable> newVariables;
        QStringList newFunctions;
        const QStringList lines = m_expression->result()->data().toString().split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        for (const QString& line : lines)
        {
            const qsizetype markerPosition = line.indexOf(marker);
            if (markerPosition < 0)
                continue;
            const QStringList fields = line.mid(markerPosition + marker.size()).split(QLatin1Char('|'), Qt::KeepEmptyParts);
            if (fields.size() != 4)
                continue;

            const QString name = decodeScilabText(fields.at(0));
            const QString value = decodeScilabText(fields.at(1));
            const QString type = decodeScilabText(fields.at(2));
            const QString dimensions = decodeScilabText(fields.at(3));
            if (name.isEmpty() || isScilabSystemVariable(name, type))
                continue;
            if (type == QLatin1String("function") || type == QLatin1String("fptr"))
                newFunctions.append(name);
            else
                newVariables.append(Variable(name, value, 0, type, dimensions));
        }
        setVariables(newVariables);
        setFunctions(newFunctions);
        setInitiallyPopulated();
    }

    finishUpdate();
}

void ScilabVariableModel::finishUpdate()
{
    if (m_expression)
    {
        m_expression->deleteLater();
        m_expression = nullptr;
    }

    if (std::exchange(m_updatePending, false))
        QTimer::singleShot(0, this, &ScilabVariableModel::update);
}

Cantor::VariablePreviewData::Reference ScilabVariableModel::variablePreview(const QModelIndex& index) const
{
    auto reference = DefaultVariableModel::variablePreview(index);
    if (!index.isValid())
        return reference;

    const auto modelVariables = variables();
    const auto& variable = modelVariables.at(index.row());
    const ScilabValue value = parseScilabValue(variable.value);
    const QStringList dimensions = variable.dimension.split(QLatin1Char('x'), Qt::SkipEmptyParts);
    const bool multidimensional = dimensions.size() > 2;
    if (multidimensional || value.kind == ScilabValue::Kind::List || value.kind == ScilabValue::Kind::Matrix || value.kind == ScilabValue::Kind::Dictionary)
    {
        reference.type = value.kind == ScilabValue::Kind::Dictionary ? Cantor::VariablePreviewData::Type::Dictionary : Cantor::VariablePreviewData::Type::Table;
        reference.backendData = scilabReferenceData({});
    }
    return reference;
}

Cantor::VariablePreviewRequest* ScilabVariableModel::requestVariablePreview(const Cantor::VariablePreviewData::Reference& reference, qsizetype offset, qsizetype limit, QObject* parent)
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

    const QStringList dimensions = variable->dimension.split(QLatin1Char('x'), Qt::SkipEmptyParts);
    if (dimensions.size() > 2 && QJsonDocument::fromJson(reference.backendData).object().value(QLatin1String("path")).toArray().isEmpty())
    {
        QTimer::singleShot(0, request, [request, dimensions]() {
            request->fail(i18n("Preview is only available for one- and two-dimensional structures. The current dimensions are %1.", dimensions.join(QStringLiteral(" x "))));
        });
        return request;
    }

    ScilabValue value = parseScilabValue(variable->value);
    QList<int> path;
    for (const auto& pathValue : QJsonDocument::fromJson(reference.backendData).object().value(QLatin1String("path")).toArray())
    {
        const int index = pathValue.toInt(-1);
        if (index < 0 || index >= value.items.size())
        {
            QTimer::singleShot(0, request, [request]() {
                request->fail(i18n("The variable or nested value no longer exists."));
            });
            return request;
        }
        path.append(index);
        value = value.items.at(index);
    }

    if (value.kind == ScilabValue::Kind::Scalar)
    {
        QTimer::singleShot(0, request, [request]() {
            request->fail(i18n("Preview is not supported for this variable type."));
        });
        return request;
    }

    auto data = scilabPreviewData(value, reference.variableName, reference.displayName, path, qMax<qsizetype>(0, offset), qMax<qsizetype>(1, limit));
    QTimer::singleShot(0, request, [request, data = std::move(data)]() mutable {
        request->complete(std::move(data));
    });
    return request;
}
