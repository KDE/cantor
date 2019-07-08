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
    std::string variables(bool parseValue) const;

  private:
    PyObject* m_pModule{nullptr};
    bool m_error{false};
    std::string filePath;
};

#endif
