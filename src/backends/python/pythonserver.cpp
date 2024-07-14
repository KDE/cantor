/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2015 Minh Ngo <minh@fedoraproject.org>
    SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
*/

#include "pythonserver.h"
#include <vector>
#include <cassert>
#include <iostream>

#include <Python.h>

static_assert(PY_MAJOR_VERSION == 3, "This python server works only with Python 3");

using namespace std;

namespace
{
    string pyObjectToQString(PyObject* obj)
    {
        return string(PyUnicode_AsUTF8(obj));
    }
}

void PythonServer::login()
{
    Py_InspectFlag = 1;
    Py_Initialize();
    m_pModule = PyImport_AddModule("__main__");
    PyRun_SimpleString("import sys");

    if (PyErr_Occurred())
    {
        m_error = true;
        PyErr_PrintEx(0);
    }

    filePath = "python_cantor_worksheet";
}

void PythonServer::interrupt()
{
    PyErr_SetInterrupt();
}

void PythonServer::runPythonCommand(const string& command)
{
    PyObject* py_dict = PyModule_GetDict(m_pModule);
    m_error = false;

    const char* prepareCommand =
        "import sys;\n"\
        "class CatchOutPythonBackend:\n"\
        "  def __init__(self, std_stream):\n"\
        "    self.value = ''\n"\
        "    self.encoding = std_stream.encoding\n"\
        "  def flush():\n"\
        "    pass\n"\
        "  def write(self, txt):\n"\
        "    self.value += txt\n"\
        "outputPythonBackend = CatchOutPythonBackend(sys.stdout)\n"\
        "errorPythonBackend  = CatchOutPythonBackend(sys.stderr)\n"\
        "sys.stdout = outputPythonBackend\n"\
        "sys.stderr = errorPythonBackend\n";
    PyRun_SimpleString(prepareCommand);

    PyObject* compile = Py_CompileString(command.c_str(), filePath.c_str(), Py_single_input);
    // There are two reasons for the error:
    // 1) This code is not single expression, so we can't compile this with flag Py_single_input
    // 2) There are errors in the code
    if (PyErr_Occurred())
    {
        PyErr_Clear();
        // Try to recompile code as sequence of expressions
        compile = Py_CompileString(command.c_str(), filePath.c_str(), Py_file_input);
        if (PyErr_Occurred())
        {
            // We now know, that we have a syntax error, so print the traceback and exit
            m_error = true;
            PyErr_PrintEx(0);
            return;
        }
    }
    PyEval_EvalCode(compile, py_dict, py_dict);

    if (PyErr_Occurred())
    {
        m_error = true;
        PyErr_PrintEx(0);
    }
}

string PythonServer::getError() const
{
    PyObject *errorPython = PyObject_GetAttrString(m_pModule, "errorPythonBackend");
    PyObject *error = PyObject_GetAttrString(errorPython, "value");

    return pyObjectToQString(error);
}

string PythonServer::getOutput() const
{
    PyObject *outputPython = PyObject_GetAttrString(m_pModule, "outputPythonBackend");
    PyObject *output = PyObject_GetAttrString(outputPython, "value");

    return pyObjectToQString(output);
}

void PythonServer::setFilePath(const string& path, const string& dir)
{
    if (path.length() != 0)
    {
        this->filePath = path;
        PyRun_SimpleString(("import sys; sys.path.insert(0, '" + dir + "')").c_str());
        PyRun_SimpleString(("__file__ = '" + path + "'").c_str());
        PyRun_SimpleString(("import os; os.chdir('" + dir + "')").c_str());
    }
}

string PythonServer::variables(bool parseValue)
{
    PyRun_SimpleStringFlags(
        "try: \n"
        "   import numpy \n"
        "   __cantor_numpy_internal__ = numpy.get_printoptions()['threshold'] \n"
        "   numpy.set_printoptions(threshold=100000000) \n"
        "except ModuleNotFoundError: \n"
        "   pass \n", nullptr
    );

    PyObject* py_dict = PyModule_GetDict(m_pModule);
    PyRun_SimpleString("__tmp_globals__ = globals()");
    PyObject* globals = PyObject_GetAttrString(m_pModule,"__tmp_globals__");
    PyObject *key, *value;
    Py_ssize_t pos = 0;

    vector<string> vars;
    while (PyDict_Next(globals, &pos, &key, &value)) {
        const string& keyString = pyObjectToQString(key);
        if (keyString.substr(0, 2) == string("__"))
            continue;

        if (keyString == string("CatchOutPythonBackend")
            || keyString == string("errorPythonBackend")
            || keyString == string("outputPythonBackend"))
            continue;

        if (PyModule_Check(value))
            continue;

        if (PyFunction_Check(value))
            continue;

        if (PyType_Check(value))
            continue;

        string valueString;
        string sizeString;
        string typeString;
        if (parseValue)
        {
            valueString = pyObjectToQString(PyObject_Repr(value));

            string command = "sys.getsizeof(" + keyString + ")";
            sizeString = pyObjectToQString(PyObject_Repr(PyRun_String(command.c_str(), Py_eval_input, py_dict, py_dict)));

            command = "type(" + keyString + ")";
            typeString = pyObjectToQString(PyObject_Repr(PyRun_String(command.c_str(), Py_eval_input, py_dict, py_dict)));
        }

        vars.push_back(keyString + char(17) + valueString + char(17) + sizeString + char(17) + typeString);
    }

    PyRun_SimpleStringFlags(
        "try: \n"
        "   import numpy \n"
        "   numpy.set_printoptions(threshold=__cantor_numpy_internal__) \n"
        "   del __cantor_numpy_internal__ \n"
        "except ModuleNotFoundError: \n"
        "   pass \n", nullptr
    );

    string result;
    for (const string& s : vars)
        result += s + char(18);
    result += char(18);
    return result;
}

bool PythonServer::isError() const
{
    return m_error;
}
