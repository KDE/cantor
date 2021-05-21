/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2015 Minh Ngo <minh@fedoraproject.org>
*/

#ifndef _PYTHONSERVER_H
#define _PYTHONSERVER_H
#include <string>

struct _object;
using PyObject = _object;

class PythonServer
{
  public:
    explicit PythonServer() = default;

  public:
    void login();
    void interrupt();
    void setFilePath(const std::string& path, const std::string& dir);
    void runPythonCommand(const std::string& command);
    std::string getOutput() const;
    std::string getError() const;
    bool isError() const;
    std::string variables(bool parseValue);

  private:
    PyObject* m_pModule{nullptr};
    bool m_error{false};
    std::string filePath;
};

#endif
