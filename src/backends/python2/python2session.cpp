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
    Copyright (C) 2015 Minh Ngo <minh@fedoraproject.org>
 */

#include "python2session.h"
#include "settings.h"

#include <Python.h>

Python2Session::Python2Session(Cantor::Backend* backend)
    : PythonSession(backend)
    , m_pModule(nullptr)
{
}

void Python2Session::runPythonCommand(const QString& command) const
{
    PyRun_SimpleString(command.toStdString().c_str());
}

void Python2Session::login()
{
    Py_Initialize();
    m_pModule = PyImport_AddModule("__main__");

    PythonSession::login();
}

QString Python2Session::getOutput() const
{
    PyObject *outputPython = PyObject_GetAttrString(m_pModule, "outputPythonBackend");
    PyObject *output = PyObject_GetAttrString(outputPython, "value");

    return pyObjectToQString(output);
}

QString Python2Session::getError() const
{
    PyObject *errorPython = PyObject_GetAttrString(m_pModule, "errorPythonBackend");
    PyObject *error = PyObject_GetAttrString(errorPython, "value");

    return pyObjectToQString(error);
}

QString Python2Session::pyObjectToQString(PyObject* obj) const
{
    return QString::fromLocal8Bit(PyString_AsString(obj));
}

bool Python2Session::integratePlots() const
{
    return PythonSettings::integratePlots();
}

QStringList Python2Session::autorunScripts() const
{
    return PythonSettings::autorunScripts();
}