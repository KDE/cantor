#include "luavariablemodel.h"
#include "result.h"

#include <KLocalizedString>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QTimer>

#include <algorithm>
#include <utility>

namespace
{

QString jsonValueText(const QJsonValue& value)
{
    if (value.isString())
        return value.toString();
    if (value.isBool())
        return value.toBool() ? QStringLiteral("true") : QStringLiteral("false");
    if (value.isDouble())
        return QString::number(value.toDouble(), 'g', 15);
    if (value.isNull())
        return QStringLiteral("nil");

    QByteArray json;
    if (value.isArray())
        json = QJsonDocument(value.toArray()).toJson(QJsonDocument::Compact);
    else if (value.isObject())
        json = QJsonDocument(value.toObject()).toJson(QJsonDocument::Compact);
    else
        return QString();

    QString result = QString::fromUtf8(json);
    constexpr qsizetype maximumLength = 200;
    if (result.size() > maximumLength)
        result = result.left(maximumLength - 3) + QStringLiteral("...");
    return result;
}

QString jsonValueType(const QJsonValue& value)
{
    if (value.isString())
        return QStringLiteral("string");
    if (value.isBool())
        return QStringLiteral("boolean");
    if (value.isDouble())
        return QStringLiteral("number");
    if (value.isNull())
        return QStringLiteral("nil");
    return QStringLiteral("table");
}

QByteArray luaReferenceData(const QJsonArray& path)
{
    QJsonObject object;
    object.insert(QLatin1String("path"), path);
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

Cantor::VariablePreviewData::Cell luaCell(const QString& value, const QString& type = QString())
{
    Cantor::VariablePreviewData::Cell cell;
    cell.value = value;
    cell.type = type;
    return cell;
}

bool isScalarMatrix(const QJsonArray& array, qsizetype* columnCount)
{
    if (array.isEmpty() || !array.first().isArray())
        return false;

    const qsizetype columns = array.first().toArray().size();
    for (const auto& rowValue : array)
    {
        if (!rowValue.isArray() || rowValue.toArray().size() != columns)
            return false;
        for (const auto& cell : rowValue.toArray())
            if (cell.isArray() || cell.isObject())
                return false;
    }
    *columnCount = columns;
    return true;
}

Cantor::VariablePreviewData luaPreviewData(const QJsonValue& value, const QString& variableName, const QString& displayName, const QJsonArray& path, qsizetype offset, qsizetype limit)
{
    using PreviewData = Cantor::VariablePreviewData;
    PreviewData data;
    data.type = value.isObject() ? PreviewData::Type::Dictionary : PreviewData::Type::Table;
    data.typeName = value.isObject() ? QStringLiteral("table (dictionary)") : QStringLiteral("table");
    data.offset = offset;

    if (value.isObject())
    {
        const QJsonObject object = value.toObject();
        const QStringList keys = object.keys();
        data.totalRows = keys.size();
        data.dimensions = QString::number(data.totalRows);
        data.columnNames = {QStringLiteral("@key"), QStringLiteral("@type"), QStringLiteral("@value")};
        for (qsizetype index = offset; index < qMin(offset + limit, data.totalRows); ++index)
        {
            const QString& key = keys.at(index);
            const QJsonValue item = object.value(key);
            PreviewData::Cell valueCell = luaCell(jsonValueText(item), jsonValueType(item));
            if (item.isArray() || item.isObject())
            {
                QJsonArray childPath = path;
                QJsonObject step;
                step.insert(QLatin1String("key"), key);
                childPath.append(step);
                valueCell.reference.variableName = variableName;
                valueCell.reference.displayName = QStringLiteral("%1[%2]").arg(displayName, key);
                valueCell.reference.backendData = luaReferenceData(childPath);
                valueCell.reference.type = item.isObject() ? PreviewData::Type::Dictionary : PreviewData::Type::Table;
            }
            data.rows.append({luaCell(key), luaCell(jsonValueType(item)), std::move(valueCell)});
        }
    }
    else
    {
        const QJsonArray array = value.toArray();
        qsizetype columnCount = 0;
        const bool matrix = isScalarMatrix(array, &columnCount);
        data.totalRows = array.size();
        if (matrix)
        {
            data.dimensions = QStringLiteral("%1x%2").arg(data.totalRows).arg(columnCount);
            data.columnNames.append(QStringLiteral("@index"));
            for (qsizetype column = 0; column < columnCount; ++column)
                data.columnNames.append(QString::number(column + 1));
            for (qsizetype rowIndex = offset; rowIndex < qMin(offset + limit, data.totalRows); ++rowIndex)
            {
                QList<PreviewData::Cell> row{luaCell(QString::number(rowIndex + 1), QStringLiteral("number"))};
                for (const auto& item : array.at(rowIndex).toArray())
                    row.append(luaCell(jsonValueText(item), jsonValueType(item)));
                data.rows.append(std::move(row));
            }
        }
        else
        {
            data.dimensions = QString::number(data.totalRows);
            data.columnNames = {QStringLiteral("@index"), QStringLiteral("@type"), QStringLiteral("@value")};
            for (qsizetype index = offset; index < qMin(offset + limit, data.totalRows); ++index)
            {
                const QJsonValue item = array.at(index);
                PreviewData::Cell valueCell = luaCell(jsonValueText(item), jsonValueType(item));
                if (item.isArray() || item.isObject())
                {
                    QJsonArray childPath = path;
                    QJsonObject step;
                    step.insert(QLatin1String("index"), index);
                    childPath.append(step);
                    valueCell.reference.variableName = variableName;
                    valueCell.reference.displayName = QStringLiteral("%1[%2]").arg(displayName).arg(index + 1);
                    valueCell.reference.backendData = luaReferenceData(childPath);
                    valueCell.reference.type = item.isObject() ? PreviewData::Type::Dictionary : PreviewData::Type::Table;
                }
                data.rows.append({luaCell(QString::number(index + 1), QStringLiteral("number")), luaCell(jsonValueType(item)), std::move(valueCell)});
            }
        }
    }

    data.hasMore = offset + limit < data.totalRows;
    return data;
}

QString luaStringLiteral(QString value)
{
    value.replace(QLatin1Char('\\'), QStringLiteral("\\\\"));
    value.replace(QLatin1Char('"'), QStringLiteral("\\\""));
    value.replace(QLatin1Char('\n'), QStringLiteral("\\n"));
    value.replace(QLatin1Char('\r'), QStringLiteral("\\r"));
    return QLatin1Char('"') + value + QLatin1Char('"');
}

}

LuaVariableModel::LuaVariableModel(Cantor::Session* session) : Cantor::DefaultVariableModel(session)
{
}

LuaVariableModel::~LuaVariableModel() = default;

void LuaVariableModel::initialize()
{
    if (m_expression || !m_baselineNames.isEmpty())
        return;

    m_initializing = true;
    update();
}

void LuaVariableModel::update()
{
    if (m_expression)
    {
        m_updatePending = true;
        return;
    }

    m_updatePending = false;

    if (m_initializing)
    {
        static const QString initializationCommand = QStringLiteral(R"LUA(
            do
            local function __cantor_hex(value)
            return (value:gsub('.', function(character) return string.format('%02x', string.byte(character)) end))
            end
            local variables, functions = {}, {}
            for name, value in pairs(_G) do
            if type(name) == 'string' then
                if type(value) == 'function' then functions[#functions + 1] = __cantor_hex(name)
                else variables[#variables + 1] = __cantor_hex(name) end
            end
            end
            table.sort(variables)
            table.sort(functions)
            print(table.concat(variables, '\30') .. '\31' .. table.concat(functions, '\30'))
            end
            )LUA");
        m_expression = session()->evaluateExpression(scriptCommand(initializationCommand), Cantor::Expression::FinishingBehavior::DoNotDelete, true);
        connect(m_expression, &Cantor::Expression::statusChanged, this, &LuaVariableModel::parseResult);
        return;
    }

    const QString introspectCommandTemplate = QStringLiteral(R"LUA(
        do
        local function __cantor_hex(value)
        return (value:gsub('.', function(character) return string.format('%02x', string.byte(character)) end))
        end
        local function __cantor_json_string(value)
        return '"' .. value:gsub('[%z\1-\31\\"]', function(character)
            local escapes = {['"']='\\"', ['\\']='\\\\', ['\b']='\\b', ['\f']='\\f', ['\n']='\\n', ['\r']='\\r', ['\t']='\\t'}
            return escapes[character] or string.format('\\u%04x', string.byte(character))
        end) .. '"'
        end
        local function __cantor_json(value, seen)
        local value_type = type(value)
        if value_type == 'string' then return __cantor_json_string(value) end
        if value_type == 'boolean' then return tostring(value) end
        if value_type == 'number' then
            return value == value and value ~= math.huge and value ~= -math.huge and tostring(value) or __cantor_json_string(tostring(value))
        end
        if value_type ~= 'table' then return __cantor_json_string('<' .. value_type .. '>') end
        if seen[value] then return __cantor_json_string('<cycle>') end
        seen[value] = true
        local count, maximum, is_array = 0, 0, true
        for key in pairs(value) do
            count = count + 1
            if type(key) ~= 'number' or key < 1 or key % 1 ~= 0 then is_array = false else maximum = math.max(maximum, key) end
        end
        is_array = is_array and maximum == count
        local parts = {}
        if is_array then
            for index = 1, count do parts[#parts + 1] = __cantor_json(value[index], seen) end
            seen[value] = nil
            return '[' .. table.concat(parts, ',') .. ']'
        end
        local keys = {}
        for key in pairs(value) do keys[#keys + 1] = key end
        table.sort(keys, function(left, right) return tostring(left) < tostring(right) end)
        for _, key in ipairs(keys) do parts[#parts + 1] = __cantor_json_string(tostring(key)) .. ':' .. __cantor_json(value[key], seen) end
        seen[value] = nil
        return '{' .. table.concat(parts, ',') .. '}'
        end
        local baseline = {__CANTOR_BASELINE__}
        local variables, functions = {}, {}
        for name, value in pairs(_G) do
        local value_type = type(value)
        if value_type == 'function' and not baseline[name] then
            functions[#functions + 1] = __cantor_hex(name)
        elseif type(name) == 'string' and not name:match('^_') and not baseline[name] then
            local serialized = __cantor_json(value, {})
            local dimensions = value_type == 'table' and tostring(#value) or ''
            variables[#variables + 1] = __cantor_hex(name) .. '\29' .. __cantor_hex(serialized) .. '\29' .. value_type .. '\29' .. dimensions
        end
        end
        table.sort(variables)
        table.sort(functions)
        print(table.concat(variables, '\30') .. '\31' .. table.concat(functions, '\30'))
        end
        )LUA");

    QStringList baselineEntries;
    baselineEntries.reserve(m_baselineNames.size());
    for (const QString& name : std::as_const(m_baselineNames))
        baselineEntries.append(QStringLiteral("[%1]=true").arg(luaStringLiteral(name)));
    QString introspectCommand = introspectCommandTemplate;
    introspectCommand.replace(QLatin1String("__CANTOR_BASELINE__"), baselineEntries.join(QLatin1Char(',')));

    m_expression = session()->evaluateExpression(scriptCommand(introspectCommand), Cantor::Expression::FinishingBehavior::DoNotDelete, true);
    connect(m_expression, &Cantor::Expression::statusChanged, this, &LuaVariableModel::parseResult);
}

void LuaVariableModel::parseResult(Cantor::Expression::Status status)
{
    if (status == Cantor::Expression::Status::Computing || status == Cantor::Expression::Status::Queued)
        return;

    if (status != Cantor::Expression::Status::Done || !m_expression || !m_expression->result())
    {
        m_initializing = false;
        finishUpdate();
        return;
    }

    const QString data = m_expression->result()->data().toString();
    const QStringList parts = data.trimmed().split(QChar(31), Qt::KeepEmptyParts);
    if (parts.size() >= 2)
    {
        if (m_initializing)
        {
            for (const QString& name : parts[0].split(QChar(30), Qt::SkipEmptyParts))
                m_baselineNames.insert(QString::fromUtf8(QByteArray::fromHex(name.toLatin1())));
            for (const QString& name : parts[1].split(QChar(30), Qt::SkipEmptyParts))
            {
                const QString decodedName = QString::fromUtf8(QByteArray::fromHex(name.toLatin1()));
                m_baselineNames.insert(decodedName);
                m_baselineFunctions.append(decodedName);
            }
            setFunctions(m_baselineFunctions);
            m_initializing = false;
            setInitiallyPopulated();
            finishUpdate();
            return;
        }

        QList<Variable> newVariables;
        const QStringList records = parts[0].split(QChar(30), Qt::SkipEmptyParts);
        for (const QString& record : records)
        {
            const QStringList fields = record.split(QChar(29), Qt::KeepEmptyParts);
            if (fields.size() < 4)
                continue;
            newVariables.append(Variable(QString::fromUtf8(QByteArray::fromHex(fields.at(0).toLatin1())), QString::fromUtf8(QByteArray::fromHex(fields.at(1).toLatin1())), 0, fields.at(2), fields.at(3)));
        }
        QStringList functions = m_baselineFunctions;
        for (const QString& function : parts[1].split(QChar(30), Qt::SkipEmptyParts))
            functions.append(QString::fromUtf8(QByteArray::fromHex(function.toLatin1())));
        setVariables(newVariables);
        setFunctions(functions);
    }

    setInitiallyPopulated();
    finishUpdate();
}

QString LuaVariableModel::scriptCommand(const QString& source)
{
    m_scriptFile = std::make_unique<QTemporaryFile>(QDir::tempPath() + QStringLiteral("/cantor-lua-variables-XXXXXX.lua"));
    if (!m_scriptFile->open() || m_scriptFile->write(source.toUtf8()) < 0 || !m_scriptFile->flush())
    {
        m_scriptFile.reset();
        return QStringLiteral("error('Cantor could not create the variable introspection script')");
    }

    return QStringLiteral("dofile(%1)").arg(luaStringLiteral(m_scriptFile->fileName()));
}

void LuaVariableModel::finishUpdate()
{
    if (m_expression)
    {
        m_expression->deleteLater();
        m_expression = nullptr;
    }
    m_scriptFile.reset();

    if (std::exchange(m_updatePending, false))
        QTimer::singleShot(0, this, &LuaVariableModel::update);
}

Cantor::VariablePreviewData::Reference LuaVariableModel::variablePreview(const QModelIndex& index) const
{
    auto reference = DefaultVariableModel::variablePreview(index);
    if (!index.isValid())
        return reference;

    const QJsonDocument document = QJsonDocument::fromJson(variables().at(index.row()).value.toUtf8());
    if (document.isArray() || document.isObject())
    {
        reference.type = document.isObject() ? Cantor::VariablePreviewData::Type::Dictionary : Cantor::VariablePreviewData::Type::Table;
        reference.backendData = luaReferenceData({});
    }
    return reference;
}

Cantor::VariablePreviewRequest* LuaVariableModel::requestVariablePreview(const Cantor::VariablePreviewData::Reference& reference, qsizetype offset, qsizetype limit, QObject* parent)
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

    const QJsonDocument valueDocument = QJsonDocument::fromJson(variable->value.toUtf8());
    QJsonValue value = valueDocument.isObject() ? QJsonValue(valueDocument.object()) : QJsonValue(valueDocument.array());
    const QJsonArray path = QJsonDocument::fromJson(reference.backendData).object().value(QLatin1String("path")).toArray();
    for (const auto& pathValue : path)
    {
        const QJsonObject step = pathValue.toObject();
        if (step.contains(QLatin1String("key")) && value.isObject())
            value = value.toObject().value(step.value(QLatin1String("key")).toString());
        else if (step.contains(QLatin1String("index")) && value.isArray())
        {
            const QJsonArray array = value.toArray();
            const int index = step.value(QLatin1String("index")).toInt(-1);
            value = index >= 0 && index < array.size() ? array.at(index) : QJsonValue();
        }
        else
            value = QJsonValue();
    }

    if (!value.isArray() && !value.isObject())
    {
        QTimer::singleShot(0, request, [request]() {
            request->fail(i18n("The variable or nested value no longer exists."));
        });
        return request;
    }

    auto data = luaPreviewData(value, reference.variableName, reference.displayName, path, qMax<qsizetype>(0, offset), qMax<qsizetype>(1, limit));
    QTimer::singleShot(0, request, [request, data = std::move(data)]() mutable {
        request->complete(std::move(data));
    });
    return request;
}
