/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018 Nikita Sirgienko <warquark@gmail.com>
*/

#include "juliavariablemodel.h"
#include "juliaextensions.h"
#include "juliasession.h"

#include <QDebug>
#include <QDBusReply>
#include <QDBusInterface>
#include <QString>

#include "settings.h"

using namespace Cantor;

const QRegularExpression JuliaVariableModel::typeVariableInfo = QRegularExpression(QLatin1String("\\w+\\["));
const QStringList JuliaVariableModel::internalCantorJuliaVariables = {QLatin1String("__cantor_gr_gks_need_restore__")};

JuliaVariableModel::JuliaVariableModel(JuliaSession* session):
    DefaultVariableModel(session),
    m_interface(nullptr)
{
}

void JuliaVariableModel::setJuliaServer(QDBusInterface* interface)
{
    m_interface = interface;
}

void JuliaVariableModel::update()
{
    if (!m_interface)
        return;

    m_interface->call(QLatin1String("parseModules"), JuliaSettings::variableManagement());

    const QStringList& variables =
        static_cast<QDBusReply<QStringList>>(m_interface->call(QLatin1String("variablesList"))).value();

    QList<Variable> vars;
    if (JuliaSettings::variableManagement())
    {
        const QStringList& values =
            static_cast<QDBusReply<QStringList>>(m_interface->call(QLatin1String("variableValuesList"))).value();
        const QStringList& variablesSizes =
            static_cast<QDBusReply<QStringList>>(m_interface->call(QLatin1String("variableSizesList"))).value();

        for (int i = 0; i < variables.size(); i++)
        {
            if (i >= values.size())
            {
                qWarning() << "Don't have value for variable from julia server response, something wrong!";
                continue;
            }

            const QString& name = variables[i];
            QString value = values[i];
            size_t size = variablesSizes[i].toULongLong();
            if (!internalCantorJuliaVariables.contains(name) && value != JuliaVariableManagementExtension::REMOVED_VARIABLE_MARKER)
            {
                // Register variable
                // We use replace here, because julia return data type for some variables, and we need
                // remove it to make variable view more consistent with the other backends
                // More info: https://bugs.kde.org/show_bug.cgi?id=377771
                vars << Variable(name, value.replace(typeVariableInfo, QLatin1String("[")), size);
            }
        }
    }
    else
    {
        for (int i = 0; i < variables.size(); i++)
            vars << Variable(variables[i], QString());
    }
    setVariables(vars);

    const QStringList& newFuncs =
        static_cast<QDBusReply<QStringList>>(m_interface->call(QLatin1String("functionsList"))).value();
    setFunctions(newFuncs);
}
