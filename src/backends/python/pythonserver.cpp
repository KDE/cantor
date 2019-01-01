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

#include "pythonserver.h"

#include <Python.h>

PythonServer::PythonServer(QObject* parent) : QObject(parent), m_pModule(nullptr)
{
}

namespace
{
    QString pyObjectToQString(PyObject* obj)
    {
#if PY_MAJOR_VERSION == 3
        return QString::fromUtf8(PyUnicode_AsUTF8(obj));
#elif PY_MAJOR_VERSION == 2
        return QString::fromLocal8Bit(PyString_AsString(obj));
#else
    #warning Unknown Python version
#endif
    }
}

void PythonServer::login()
{
    Py_InspectFlag = 1;
    Py_Initialize();
    m_pModule = PyImport_AddModule("__main__");
    filePath = QLatin1String("python_cantor_worksheet");
}

void PythonServer::runPythonCommand(const QString& command) const
{
    PyObject* py_dict = PyModule_GetDict(m_pModule);

    const char* prepareCommand =
        "import sys;\n"\
        "class CatchOutPythonBackend:\n"\
        "  def __init__(self):\n"\
        "    self.value = ''\n"\
        "  def write(self, txt):\n"\
        "    self.value += txt\n"\
        "outputPythonBackend = CatchOutPythonBackend()\n"\
        "errorPythonBackend  = CatchOutPythonBackend()\n"\
        "sys.stdout = outputPythonBackend\n"\
        "sys.stderr = errorPythonBackend\n";
    PyRun_SimpleString(prepareCommand);

#if PY_MAJOR_VERSION == 3
    PyObject* compile = Py_CompileString(command.toStdString().c_str(), filePath.toStdString().c_str(), Py_single_input);
    // There are two reasons for the error:
    // 1) This code is not single expression, so we can't compile this with flag Py_single_input
    // 2) There are errors in the code
    if (PyErr_Occurred())
    {
        PyErr_Clear();
        // Try to recompile code as sequence of expressions
        compile = Py_CompileString(command.toStdString().c_str(), filePath.toStdString().c_str(), Py_file_input);
        if (PyErr_Occurred())
        {
            // We now know, that we have a syntax error, so print the traceback and exit
            PyErr_PrintEx(0);
            return;
        }
    }
    PyEval_EvalCode(compile, py_dict, py_dict);
#elif PY_MAJOR_VERSION == 2
    // Python 2.X don't check, that input string contains only one expression.
    // So for checking this, we compile string as file and as single expression and compare bytecode
    // FIXME?
    PyObject* codeFile = Py_CompileString(command.toStdString().c_str(), filePath.toStdString().c_str(), Py_file_input);
    if (PyErr_Occurred())
    {
        PyErr_PrintEx(0);
        return;
    }
    PyObject* codeSingle = Py_CompileString(command.toStdString().c_str(), filePath.toStdString().c_str(), Py_single_input);
    if (PyErr_Occurred())
    {
        // We have error with Py_single_input, but haven't error with Py_file_input
        // So, the code can't be compiled as singel input -> use file input right away
        PyErr_Clear();
        PyEval_EvalCode((PyCodeObject*)codeFile, py_dict, py_dict);
    }
    else
    {
        PyObject* bytecode1 = ((PyCodeObject*)codeSingle)->co_code;
        PyObject* bytecode2 = ((PyCodeObject*)codeFile)->co_code;

        if (PyObject_Length(bytecode1) >= PyObject_Length(bytecode2))
        {
            PyEval_EvalCode((PyCodeObject*)codeSingle, py_dict, py_dict);
        }
        else
        {
            PyEval_EvalCode((PyCodeObject*)codeFile, py_dict, py_dict);
        }
    }
#else
    #warning Unknown Python version
#endif
    if (PyErr_Occurred())
        PyErr_PrintEx(0);
}

QString PythonServer::getError() const
{
    PyObject *errorPython = PyObject_GetAttrString(m_pModule, "errorPythonBackend");
    PyObject *error = PyObject_GetAttrString(errorPython, "value");

    return pyObjectToQString(error);
}

QString PythonServer::getOutput() const
{
    PyObject *outputPython = PyObject_GetAttrString(m_pModule, "outputPythonBackend");
    PyObject *output = PyObject_GetAttrString(outputPython, "value");

    return pyObjectToQString(output);
}

void PythonServer::setFilePath(const QString& path)
{
    this->filePath = path;
    PyRun_SimpleString(("__file__ = '"+path.toStdString()+"'").c_str());
}


