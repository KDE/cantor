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
#include <QObject>
#include <QString>

struct _object;
typedef _object PyObject;

class PythonServer : public QObject
{
  Q_OBJECT
  public:
    PythonServer(QObject* parent = nullptr);

  public Q_SLOTS:
    Q_SCRIPTABLE void login();
    Q_SCRIPTABLE void runPythonCommand(const QString& command) const;
    Q_SCRIPTABLE QString getOutput() const;
    Q_SCRIPTABLE QString getError() const;

  private:
    PyObject* m_pModule;
};

#endif
