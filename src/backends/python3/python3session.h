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

#ifndef _PYTHON3SESSION_H
#define _PYTHON3SESSION_H

#include "../python/pythonsession.h"

class QDBusInterface;
class KProcess;
class Python3Session : public PythonSession
{
  public:
    Python3Session(Cantor::Backend* backend);

    void login();
    void logout();
    void interrupt();

    bool integratePlots() const;
    QStringList autorunScripts() const;

  private:
    void runPythonCommand(const QString& command) const;
    QString getOutput() const;
    QString getError() const;

  private:
    QDBusInterface* m_pIface;
    KProcess* m_pProcess;
};

#endif