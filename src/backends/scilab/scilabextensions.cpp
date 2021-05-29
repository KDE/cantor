/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2013 Filipe Saraiva <filipe@kde.org>
*/

#include "scilabextensions.h"
#include <KLocalizedString>

#include <QDebug>

#define SCILAB_EXT_CDTOR(name) Scilab##name##Extension::Scilab##name##Extension(QObject* parent) : name##Extension(parent) {} \
                                     Scilab##name##Extension::~Scilab##name##Extension() {}

SCILAB_EXT_CDTOR(Script)

QString ScilabScriptExtension::runExternalScript(const QString& path)
{
    return QString::fromLatin1("exec(\"%1\", -1)").arg(path);
}

QString ScilabScriptExtension::scriptFileFilter()
{
    return i18n(";;Scilab script file (*.sce);;Scilab function file (*.sci)");
}

QString ScilabScriptExtension::highlightingMode()
{
    return QLatin1String("scilab");
}

QString ScilabScriptExtension::commandSeparator()
{
    return QLatin1String(";");
}

SCILAB_EXT_CDTOR(VariableManagement)

QString ScilabVariableManagementExtension::addVariable(const QString& name, const QString& value)
{
    return setValue(name,value);
}

QString ScilabVariableManagementExtension::setValue(const QString& name, const QString& value)
{
    return QString::fromLatin1("%1 = %2").arg(name).arg(value);
}

QString ScilabVariableManagementExtension::removeVariable(const QString& name)
{
    return QString::fromLatin1("clear %1;").arg(name);
}

QString ScilabVariableManagementExtension::clearVariables()
{
    return QLatin1String("clear;");
}

QString ScilabVariableManagementExtension::saveVariables(const QString& fileName)
{
    return QString::fromLatin1("save('%1');").arg(fileName);
}

QString ScilabVariableManagementExtension::loadVariables(const QString& fileName)
{
    return QString::fromLatin1("load('%1');").arg(fileName);
}
