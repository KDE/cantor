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
    Q_OBJECT
  public:
    Python3Session(Cantor::Backend* backend);

    void login() override;
    void logout() override;
    void interrupt() override;

    bool integratePlots() const override;
    QStringList autorunScripts() const override;

  private:
    void runPythonCommand(const QString& command) const override;
    void readExpressionOutput(const QString& commandProcessing) override;

    QString getOutput() const override;
    QString getError() const override;

  private:
    QDBusInterface* m_pIface;
    KProcess* m_pProcess;

  Q_SIGNALS:
    void updateHighlighter();
};

#endif
