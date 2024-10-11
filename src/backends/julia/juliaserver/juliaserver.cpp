/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
    SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
*/
#include "juliaserver.h"

#include <julia_version.h>

#include <iostream>
#include <QFileInfo>
#include <QDir>
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
    parseJlModule(JL_MAIN_MODULE, variableManagement);
}

void JuliaServer::parseJlModule(jl_module_t* module, bool parseValue)
{
    jl_function_t* jl_string_function = jl_get_function(jl_base_module, "string");
    jl_function_t* jl_sizeof_function = jl_get_function(jl_base_module, "sizeof");

    if (module != JL_MAIN_MODULE)
    {
        const QString& moduleName = fromJuliaString(jl_call1(jl_string_function, (jl_value_t*)(module->name)));
        if (parsedModules.contains(moduleName))
            return;
        else
            parsedModules.append(moduleName);
    }

    jl_function_t* jl_names_function = jl_get_function(jl_base_module, "names");
    jl_value_t* names = jl_call1(jl_names_function, (jl_value_t*)module);
#if QT_VERSION_CHECK(JULIA_VERSION_MAJOR, JULIA_VERSION_MINOR, 0) >= QT_VERSION_CHECK(1, 11, 0)
    jl_value_t **data = (jl_value_t**)jl_array_data_(names);
#else
    jl_value_t **data = (jl_value_t**)jl_array_data(names);
#endif
    for (size_t i = 0; i < jl_array_len(names); i++)
    {
        bool isBindingResolved = (bool)jl_binding_resolved_p(module, (jl_sym_t*)(data[i]));
        if (isBindingResolved)
        {

            const auto& name = fromJuliaString(jl_call1(jl_string_function, data[i]));
            jl_value_t* value = jl_get_binding_or_error(module, (jl_sym_t*)(data[i]))->value;
            jl_datatype_t* datetype = (jl_datatype_t*)jl_typeof(value);
            const auto& type = QString::fromUtf8(jl_typeof_str(value));

            // Module
            if (jl_is_module(value))
            {
                if (module == JL_MAIN_MODULE && (jl_module_t*)value != JL_MAIN_MODULE)
                    parseJlModule((jl_module_t*)value, parseValue);
            }
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

QStringList JuliaServer::functionsList()
{
    return m_functions;
}
