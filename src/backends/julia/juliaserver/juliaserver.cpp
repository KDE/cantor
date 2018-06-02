/*
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    as published by the Free Software Foundation; either version 2
 *    of the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA  02110-1301, USA.
 *
 *    ---
 *    Copyright (C) 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
 */
#include "juliaserver.h"

#include <julia/julia.h>
#include <julia/julia_version.h>

#include <iostream>
#include <QFileInfo>
#include <QDir>
#include <QTemporaryFile>
#include <QDebug>

JuliaServer::JuliaServer(QObject *parent)
    : QObject(parent)
{
}

JuliaServer::~JuliaServer()
{
    jl_atexit_hook(0);
}

void JuliaServer::login(const QString &path) const
{
#if JULIA_VERSION_MINOR > 5
    Q_UNUSED(path)
    jl_init();
#else
    QString dir_path = QFileInfo(path).dir().absolutePath();
    jl_init(dir_path.toLatin1().constData());
#endif
}

void JuliaServer::runJuliaCommand(const QString &command)
{
    // Redirect stdout, stderr to temprorary files
    QTemporaryFile output, error;
    if (!output.open() || !error.open()) {
        qFatal("Unable to create temporary files for stdout/stderr");
        return;
    }
    jl_eval_string("const __originalSTDOUT__ = STDOUT");
    jl_eval_string("const __originalSTDERR__ = STDERR");
    jl_eval_string(
        QString::fromLatin1("redirect_stdout(open(\"%1\", \"w\"))")
            .arg(output.fileName()).toLatin1().constData()
    );
    jl_eval_string(
        QString::fromLatin1("redirect_stderr(open(\"%1\", \"w\"))")
            .arg(error.fileName()).toLatin1().constData()
    );

    // Run command
    jl_value_t *val = static_cast<jl_value_t *>(
        jl_eval_string(command.toUtf8().constData())
    );

    if (jl_exception_occurred()) { // If exception occurred
        // Show it to user in stderr
        jl_value_t *ex = jl_exception_in_transit;
        jl_printf(JL_STDERR, "error during run:\n");
        jl_function_t *showerror =
            jl_get_function(jl_base_module, "showerror");
        jl_value_t *bt = static_cast<jl_value_t *>(
            jl_eval_string("catch_backtrace()")
        );
        jl_value_t *err_stream = static_cast<jl_value_t *>(
            jl_eval_string("STDERR")
        );
        jl_call3(showerror, err_stream, ex, bt);
        jl_exception_clear();
        m_was_exception = true;
    } else if (val) { // no exception occurred
        // If last result is not nothing, show it
        jl_function_t *equality = jl_get_function(jl_base_module, "==");
        jl_value_t *nothing =
            static_cast<jl_value_t *>(jl_eval_string("nothing"));
        bool is_nothing = jl_unbox_bool(
            static_cast<jl_value_t *>(jl_call2(equality, nothing, val))
        );
        if (!is_nothing) {
            jl_value_t *out_display = static_cast<jl_value_t *>(jl_eval_string("TextDisplay(STDOUT)"));
            jl_function_t *display = jl_get_function(jl_base_module, "display");
            jl_call2(display, out_display, val);
        }
        m_was_exception = false;
    }
    // Clean up streams and files
    jl_eval_string("flush(STDOUT)");
    jl_eval_string("flush(STDERR)");
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

