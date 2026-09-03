/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
    SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
*/
#include "juliaserver.h"

#include <julia_version.h>

#include <iostream>
#include <optional>
#include <QFileInfo>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryFile>
#include <QDebug>

QStringList JuliaServer::INTERNAL_VARIABLES =
    QStringList() << QLatin1String("__originalSTDOUT__") << QLatin1String("__originalSTDERR__");

JuliaServer::JuliaServer(QObject *parent) : QObject(parent), m_was_exception(false)
{
}

JuliaServer::~JuliaServer()
{
     /* strongly recommended: notify Julia that the
         program is about to terminate. this allows
         Julia time to cleanup pending write requests
         and run all finalizers
    */
    jl_atexit_hook(0);
}

int JuliaServer::login()
{
    /* required: setup the julia context */
    jl_init();

    jl_eval_string("import REPL;");

    return 0;
}

void JuliaServer::runJuliaCommand(const QString &command)
{
    // Redirect stdout, stderr to temporary files
    QTemporaryFile output, error;
    if (!output.open() || !error.open()) {
        qFatal("Unable to create temporary files for stdout/stderr");
        return;
    }
    jl_eval_string("const __originalSTDOUT__ = stdout");
    jl_eval_string("const __originalSTDERR__ = stderr");
    jl_eval_string(
        QString::fromLatin1("redirect_stdout(open(\"%1\", \"w\"))")
            .arg(output.fileName()).toLatin1().constData()
    );
    jl_eval_string(
        QString::fromLatin1("redirect_stderr(open(\"%1\", \"w\"))")
            .arg(error.fileName()).toLatin1().constData()
    );

    jl_module_t* jl_repl_module = (jl_module_t*)(jl_eval_string("REPL"));
    jl_function_t* jl_ends_func = jl_get_function(jl_repl_module, "ends_with_semicolon");
    bool isEndsWithSemicolon = jl_unbox_bool(jl_call1(jl_ends_func, jl_cstr_to_string(command.toStdString().c_str())));

    // Run command
    jl_value_t *val = static_cast<jl_value_t *>(
        jl_eval_string(command.toUtf8().constData())
    );

    if (jl_exception_occurred()) { // If exception occurred
        // Show it to user in stderr
#if QT_VERSION_CHECK(JULIA_VERSION_MAJOR, JULIA_VERSION_MINOR, 0) >= QT_VERSION_CHECK(1, 7, 0)
        jl_value_t *ex = jl_current_task->ptls->previous_exception;
#elif QT_VERSION_CHECK(JULIA_VERSION_MAJOR, JULIA_VERSION_MINOR, 0) >= QT_VERSION_CHECK(1, 1, 0)
        jl_value_t *ex = jl_get_ptls_states()->previous_exception;
#else
        jl_value_t *ex = jl_exception_in_transit;
#endif
        jl_printf(JL_STDERR, "error during run:\n");
        jl_function_t *showerror =
            jl_get_function(jl_base_module, "showerror");
        jl_value_t *bt = static_cast<jl_value_t *>(
            jl_eval_string("catch_backtrace()")
        );
        jl_value_t *err_stream = static_cast<jl_value_t *>(
            jl_eval_string("stderr")
        );
        jl_call3(showerror, err_stream, ex, bt);
        jl_exception_clear();
        m_was_exception = true;
    } else if (val && !isEndsWithSemicolon) { // no exception occurred
        // If last result is not nothing, show it
        jl_function_t *equality = jl_get_function(jl_base_module, "==");
        jl_value_t *nothing =
            static_cast<jl_value_t *>(jl_eval_string("nothing"));
        bool is_nothing = jl_unbox_bool(
            static_cast<jl_value_t *>(jl_call2(equality, nothing, val))
        );
        if (!is_nothing) {
            jl_value_t *out_display = static_cast<jl_value_t *>(jl_eval_string("TextDisplay(stdout)"));
            jl_function_t *display = jl_get_function(jl_base_module, "display");
            jl_call2(display, out_display, val);
        }
        m_was_exception = false;
    }
    // Clean up streams and files
    jl_eval_string("flush(stdout)");
    jl_eval_string("flush(stderr)");
    jl_eval_string("redirect_stdout(__originalSTDOUT__)");
    jl_eval_string("redirect_stderr(__originalSTDERR__)");

    // Clean up variables
    auto vars_to_remove = {
        "__originalSTDOUT__", "__originalSTDERR__"
    };

    for (const auto &var : vars_to_remove) {
        jl_eval_string(
            QString::fromLatin1("%1 = 0").arg(QLatin1String(var))
                .toLatin1().constData()
        );
    }

    m_output = QString::fromUtf8(output.readAll());
    m_error = QString::fromUtf8(error.readAll());
}

QString JuliaServer::getError() const
{
    return m_error;
}

QString JuliaServer::getOutput() const
{
    return m_output;
}

bool JuliaServer::getWasException() const
{
    return m_was_exception;
}

#if QT_VERSION_CHECK(JULIA_VERSION_MAJOR, JULIA_VERSION_MINOR, 0) >= QT_VERSION_CHECK(1, 1, 0)
#define JL_MAIN_MODULE jl_main_module
#else
#define JL_MAIN_MODULE jl_internal_main_module
#endif

void JuliaServer::parseModules(bool variableManagement)
{
    m_variables.clear();
    m_variableValues.clear();
    m_variableSizes.clear();
    m_variableTypes.clear();
    m_functions.clear();
    parseJlModule(JL_MAIN_MODULE, variableManagement);
}

void JuliaServer::parseJlModule(jl_module_t* module, bool parseValue)
{
    jl_function_t* jl_string_function = jl_get_function(jl_base_module, "string");
    jl_function_t* jl_sizeof_function = jl_get_function(jl_base_module, "sizeof");

    jl_function_t* jl_names_function = jl_get_function(jl_base_module, "names");
    jl_value_t* names = jl_call1(jl_names_function, (jl_value_t*)module);
    JL_GC_PUSH1(&names);
    for (size_t i = 0; i < jl_array_len(names); i++)
    {
        auto* symbol = (jl_sym_t*)jl_array_ptr_ref(names, i);
        jl_value_t* value = nullptr;
#if QT_VERSION_CHECK(JULIA_VERSION_MAJOR, JULIA_VERSION_MINOR, 0) >= QT_VERSION_CHECK(1, 12, 0)
        value = jl_get_global(module, symbol);
        const bool isBindingResolved = value != nullptr;
#else
        const bool isBindingResolved = (bool)jl_binding_resolved_p(module, symbol);
#endif
        if (isBindingResolved)
        {

            const auto& name = fromJuliaString(jl_call1(jl_string_function, (jl_value_t*)symbol));
#if QT_VERSION_CHECK(JULIA_VERSION_MAJOR, JULIA_VERSION_MINOR, 0) < QT_VERSION_CHECK(1, 12, 0)
            value = jl_get_binding_or_error(module, symbol)->value;
#endif

            jl_datatype_t* datetype = (jl_datatype_t*)jl_typeof(value);
            const auto& type = QString::fromUtf8(jl_typeof_str(value));

            // Imported modules are not user variables and are expensive to inspect.
            if (jl_is_module(value))
                continue;
            // Function
            else if (type.startsWith(QLatin1String("#")) || type == QLatin1String("Function"))
            {
                if (!m_functions.contains(name))
                    m_functions.append(name);
            }
            // Variable
            else if (datetype != jl_datatype_type) // Not type
            {
                if (module == JL_MAIN_MODULE && !INTERNAL_VARIABLES.contains(name))
                {
                    const QString& size = fromJuliaString(jl_call1(jl_string_function, jl_call1(jl_sizeof_function, value)));
                    //const QString& type = fromJuliaString(jl_call1(jl_string_function, jl_call1(jl_typeof_function, value)));
                    if (parseValue)
                    {
                        const QString& valueString = fromJuliaString(jl_call1(jl_string_function, value));
                        if (m_variables.contains(name))
                        {
                            int i = m_variables.indexOf(name);
                            m_variableValues[i] = valueString;
                            m_variableSizes[i] = size;
                            m_variableTypes[i] = type;
                        }
                        else
                        {
                            m_variables.append(name);
                            m_variableValues.append(valueString);
                            m_variableSizes.append(size);
                            m_variableTypes.append(type);
                        }
                    }
                    else
                    {
                        if (m_variables.contains(name))
                        {
                            int i = m_variables.indexOf(name);
                            m_variableSizes[i] = size;
                            m_variableTypes[i] = type;
                        }
                        else
                        {
                            m_variables.append(name);
                            m_variableSizes.append(size);
                            m_variableTypes.append(type);
                        }
                    }
                }
            }
        }
    }
    JL_GC_POP();
}

QString JuliaServer::fromJuliaString(const jl_value_t* value)
{
    return QString::fromUtf8(jl_string_data(value));
}

QStringList JuliaServer::variablesList()
{
    return m_variables;
}

QStringList JuliaServer::variableValuesList()
{
    return m_variableValues;
}

QStringList JuliaServer::variableSizesList()
{
    return m_variableSizes;
}

QStringList JuliaServer::variableTypesList()
{
    return m_variableTypes;
}

QString JuliaServer::variablePreview(const QByteArray& referenceData, qlonglong offset, qlonglong limit)
{
    const auto juliaString = [](jl_value_t* value) {
        if (!value)
            return QStringLiteral("<unavailable>");
        jl_function_t* stringFunction = jl_get_function(jl_base_module, "string");
        jl_value_t* string = jl_call1(stringFunction, value);
        if (jl_exception_occurred() || !string)
        {
            jl_exception_clear();
            return QStringLiteral("<unprintable>");
        }
        QString result = QString::fromUtf8(jl_string_data(string));
        result.replace(QLatin1Char('\n'), QLatin1Char(' '));
        return result.left(200);
    };

    const auto previewType = [](jl_value_t* value) {
        if (!value)
            return 0;
        jl_value_t* abstractDict = jl_eval_string("AbstractDict");
        jl_value_t* abstractVector = jl_eval_string("AbstractVector");
        jl_value_t* abstractMatrix = jl_eval_string("AbstractMatrix");
        const QString typeName = QString::fromUtf8(jl_typeof_str(value));
        if (jl_isa(value, abstractDict) || typeName.contains(QLatin1String("NamedTuple")))
            return 2;
        if (jl_isa(value, abstractVector) || jl_isa(value, abstractMatrix) || jl_is_tuple(value) || typeName.contains(QLatin1String("DataFrame")))
            return 1;
        return 0;
    };

    const auto length = [](jl_value_t* value) -> qlonglong {
        jl_function_t* lengthFunction = jl_get_function(jl_base_module, "length");
        jl_value_t* result = jl_call1(lengthFunction, value);
        if (jl_exception_occurred() || !result)
        {
            jl_exception_clear();
            return 0;
        }
        return jl_unbox_int64(result);
    };

    const auto getIndex = [](jl_value_t* value, qlonglong index, std::optional<qlonglong> column = std::nullopt) -> jl_value_t* {
        jl_function_t* getIndexFunction = jl_get_function(jl_base_module, "getindex");
        jl_value_t* rowValue = jl_box_int64(index + 1);
        JL_GC_PUSH1(&rowValue);
        jl_value_t* result = nullptr;
        if (column)
        {
            jl_value_t* columnValue = jl_box_int64(*column + 1);
            JL_GC_PUSH1(&columnValue);
            result = jl_call3(getIndexFunction, value, rowValue, columnValue);
            JL_GC_POP();
        }
        else
            result = jl_call2(getIndexFunction, value, rowValue);
        JL_GC_POP();
        if (jl_exception_occurred())
        {
            jl_exception_clear();
            return nullptr;
        }
        return result;
    };

    const auto dictionaryEntry = [&getIndex](jl_value_t* value, qlonglong index, bool key) -> jl_value_t* {
        jl_function_t* function = jl_get_function(jl_base_module, key ? "keys" : "values");
        jl_value_t* iterator = jl_call1(function, value);
        if (jl_exception_occurred() || !iterator)
        {
            jl_exception_clear();
            return nullptr;
        }
        jl_value_t* items = nullptr;
        JL_GC_PUSH2(&iterator, &items);
        jl_function_t* collectFunction = jl_get_function(jl_base_module, "collect");
        items = jl_call1(collectFunction, iterator);
        jl_value_t* result = getIndex(items, index);
        JL_GC_POP();
        return result;
    };

    QJsonParseError parseError;
    const QJsonDocument referenceDocument = QJsonDocument::fromJson(referenceData, &parseError);
    QJsonObject result;
    if (parseError.error != QJsonParseError::NoError || !referenceDocument.isObject())
    {
        result.insert(QLatin1String("errorCode"), QStringLiteral("invalidReference"));
        return QString::fromLatin1(QJsonDocument(result).toJson(QJsonDocument::Compact).toBase64());
    }

    const QJsonObject reference = referenceDocument.object();
    const QString variableName = reference.value(QLatin1String("name")).toString();
    const QJsonArray path = reference.value(QLatin1String("path")).toArray();
    jl_value_t* value = jl_get_global(JL_MAIN_MODULE, jl_symbol(variableName.toUtf8().constData()));
    for (const auto& pathValue : path)
    {
        const QJsonObject step = pathValue.toObject();
        const qlonglong index = step.value(QLatin1String("index")).toInteger();
        value = step.value(QLatin1String("dictionary")).toBool() ? dictionaryEntry(value, index, false) : getIndex(value, index);
        if (!value)
            break;
    }

    if (!value)
    {
        result.insert(QLatin1String("errorCode"), QStringLiteral("valueMissing"));
        return QString::fromLatin1(QJsonDocument(result).toJson(QJsonDocument::Compact).toBase64());
    }

    offset = qMax<qlonglong>(0, offset);
    limit = qMax<qlonglong>(1, limit);
    const int type = previewType(value);
    const QString typeName = QString::fromUtf8(jl_typeof_str(value));
    result.insert(QLatin1String("type"), type);
    result.insert(QLatin1String("typeName"), typeName);
    result.insert(QLatin1String("offset"), offset);
    QJsonArray columns;
    QJsonArray rows;
    qlonglong totalRows = 0;

    const auto referenceFor = [&](qlonglong index, bool dictionary, jl_value_t* item, const QString& displayName) {
        QJsonObject itemReference;
        const int itemType = previewType(item);
        if (itemType == 0)
            return itemReference;
        QJsonArray itemPath = path;
        QJsonObject step;
        step.insert(QLatin1String("index"), index);
        step.insert(QLatin1String("dictionary"), dictionary);
        itemPath.append(step);
        QJsonObject backendReference;
        backendReference.insert(QLatin1String("name"), variableName);
        backendReference.insert(QLatin1String("displayName"), displayName);
        backendReference.insert(QLatin1String("path"), itemPath);
        itemReference.insert(QLatin1String("displayName"), displayName);
        itemReference.insert(QLatin1String("backendData"), QString::fromUtf8(QJsonDocument(backendReference).toJson(QJsonDocument::Compact)));
        itemReference.insert(QLatin1String("type"), itemType);
        return itemReference;
    };

    const auto cellFor = [&](jl_value_t* item, const QJsonObject& itemReference = {}) {
        QJsonObject cell;
        cell.insert(QLatin1String("value"), juliaString(item));
        cell.insert(QLatin1String("valueType"), QString::fromUtf8(jl_typeof_str(item)));
        if (!itemReference.isEmpty())
            cell.insert(QLatin1String("reference"), itemReference);
        return cell;
    };

    const bool isDictionary = type == 2;
    const bool isDataFrame = typeName.contains(QLatin1String("DataFrame"));
    jl_value_t* sizeValue = nullptr;
    if (type == 1 && !jl_is_tuple(value))
    {
        jl_function_t* sizeFunction = jl_get_function(jl_base_module, "size");
        sizeValue = jl_call1(sizeFunction, value);
        if (jl_exception_occurred())
        {
            jl_exception_clear();
            sizeValue = nullptr;
        }
    }

    const qlonglong dimensionCount = sizeValue ? jl_nfields(sizeValue) : 1;
    if (isDictionary)
    {
        columns = {QStringLiteral("@key"), QStringLiteral("@type"), QStringLiteral("@value")};
        totalRows = length(value);
        result.insert(QLatin1String("dimensions"), QString::number(totalRows));
        for (qlonglong i = offset; i < qMin(offset + limit, totalRows); ++i)
        {
            jl_value_t* key = dictionaryEntry(value, i, true);
            jl_value_t* item = dictionaryEntry(value, i, false);
            const QString keyText = juliaString(key);
            const QString displayName = QStringLiteral("%1[%2]").arg(reference.value(QLatin1String("displayName")).toString(), keyText);
            QJsonArray row;
            QJsonObject keyCell;
            keyCell.insert(QLatin1String("value"), keyText);
            row.append(keyCell);
            QJsonObject typeCell;
            typeCell.insert(QLatin1String("value"), QString::fromUtf8(jl_typeof_str(item)));
            row.append(typeCell);
            row.append(cellFor(item, referenceFor(i, true, item, displayName)));
            rows.append(row);
        }
    }
    else if (type == 1 && dimensionCount == 2)
    {
        const qlonglong rowCount = jl_unbox_int64(jl_get_nth_field(sizeValue, 0));
        const qlonglong columnCount = jl_unbox_int64(jl_get_nth_field(sizeValue, 1));
        totalRows = rowCount;
        columns.append(QStringLiteral("@index"));
        if (isDataFrame)
        {
            jl_function_t* propertyNames = jl_get_function(jl_base_module, "propertynames");
            jl_value_t* names = propertyNames ? jl_call1(propertyNames, value) : nullptr;
            for (qlonglong column = 0; column < columnCount; ++column)
                columns.append(names ? juliaString(getIndex(names, column)) : QString::number(column + 1));
        }
        else
            for (qlonglong column = 0; column < columnCount; ++column)
                columns.append(QString::number(column + 1));
        result.insert(QLatin1String("dimensions"), QStringLiteral("%1x%2").arg(rowCount).arg(columnCount));
        for (qlonglong rowIndex = offset; rowIndex < qMin(offset + limit, totalRows); ++rowIndex)
        {
            QJsonArray row;
            QJsonObject indexCell;
            indexCell.insert(QLatin1String("value"), QString::number(rowIndex + 1));
            row.append(indexCell);
            for (qlonglong column = 0; column < columnCount; ++column)
                row.append(cellFor(getIndex(value, rowIndex, column)));
            rows.append(row);
        }
    }
    else if (type == 1)
    {
        columns = {QStringLiteral("@index"), QStringLiteral("@type"), QStringLiteral("@value")};
        totalRows = length(value);
        result.insert(QLatin1String("dimensions"), QString::number(totalRows));
        for (qlonglong i = offset; i < qMin(offset + limit, totalRows); ++i)
        {
            jl_value_t* item = getIndex(value, i);
            const QString displayName = QStringLiteral("%1[%2]").arg(reference.value(QLatin1String("displayName")).toString()).arg(i + 1);
            QJsonArray row;
            QJsonObject indexCell;
            indexCell.insert(QLatin1String("value"), QString::number(i + 1));
            row.append(indexCell);
            QJsonObject typeCell;
            typeCell.insert(QLatin1String("value"), QString::fromUtf8(jl_typeof_str(item)));
            row.append(typeCell);
            row.append(cellFor(item, referenceFor(i, false, item, displayName)));
            rows.append(row);
        }
    }
    else
        result.insert(QLatin1String("errorCode"), QStringLiteral("unsupportedType"));

    result.insert(QLatin1String("columns"), columns);
    result.insert(QLatin1String("rows"), rows);
    result.insert(QLatin1String("totalRows"), totalRows);
    result.insert(QLatin1String("hasMore"), offset + limit < totalRows);
    return QString::fromLatin1(QJsonDocument(result).toJson(QJsonDocument::Compact).toBase64());
}

QStringList JuliaServer::functionsList()
{
    return m_functions;
}
