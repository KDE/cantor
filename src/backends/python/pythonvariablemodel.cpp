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
    Copyright (C) 2018 Nikita Sirgienko <warquark@gmail.com>
*/

#include "pythonvariablemodel.h"
#include "pythonsession.h"
#include "textresult.h"

#include <QDebug>
#include <QDBusReply>
#include <QDBusInterface>
#include <QString>

using namespace Cantor;

PythonVariableModel::PythonVariableModel(PythonSession* session):
    DefaultVariableModel(session),
    m_pIface(nullptr)
{
}

void PythonVariableModel::setPythonServer(QDBusInterface* pIface)
{
    m_pIface = pIface;
}

void PythonVariableModel::update()
{
    if (!m_pIface)
        return;

    bool variableManagement = static_cast<PythonSession*>(session())->variableManagement();
    const QString& data = QDBusReply<QString>(m_pIface->call(QString::fromLatin1("variables"), variableManagement)).value();
    const QStringList& records = data.split(QChar(30), QString::SkipEmptyParts);

    QList<Variable> variables;
    for (const QString& record : records)
    {
        const QString& name = record.section(QChar(31), 0, 0);
        const QString& value = record.section(QChar(31), 1, 1);

        variables << Variable{name, value};
    }

    setVariables(variables);
}
