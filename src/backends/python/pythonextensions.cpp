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
    Copyright (C) 2013 Filipe Saraiva <filipe@kde.org>
 */

#include "pythonextensions.h"
#include <KLocale>
#include <KDebug>

#define PYTHON_EXT_CDTOR(name) Python##name##Extension::Python##name##Extension(QObject* parent) : name##Extension(parent) {} \
                                     Python##name##Extension::~Python##name##Extension() {}

PYTHON_EXT_CDTOR(Packaging)

QString PythonPackagingExtension::importPackage(const QString& package)
{
    return QString("import %1").arg(package);
}

PYTHON_EXT_CDTOR(VariableManagement)

QString PythonVariableManagementExtension::addVariable(const QString& name, const QString& value)
{
    return setValue(name, value);
}

QString PythonVariableManagementExtension::setValue(const QString& name, const QString& value)
{
    return QString("%1 = %2").arg(name).arg(value);
}

QString PythonVariableManagementExtension::removeVariable(const QString& name)
{
    return QString("del(%1)").arg(name);
}

QString PythonVariableManagementExtension::clearVariables()
{
    QString delVariablesPythonSession = "for keyPythonBackend in dir():\n"                                 \
                                        "    if (not 'PythonBackend' in keyPythonBackend)\ "               \
                                        "and (not '__' in keyPythonBackend):\n"                            \
                                        "        del(globals()[keyPythonBackend])\n"                       \
                                        "del(keyPythonBackend)\n";
    return delVariablesPythonSession;
}

QString PythonVariableManagementExtension::saveVariables(const QString& fileName)
{
    QString classSavePythonSession = "import shelve\n"                                                               \
                                     "shelvePythonBackend = shelve.open('%1', 'n')\n"                                \
                                     "for keyPythonBackend in dir():\n"                                              \
                                     "    if (not 'PythonBackend' in keyPythonBackend)\ "                            \
                                     "and (not '__' in keyPythonBackend)\ "                                          \
                                     "and (not '<module ' in unicode(globals()[keyPythonBackend])):\n"               \
                                     "        shelvePythonBackend[keyPythonBackend] = globals()[keyPythonBackend]\n" \
                                     "shelvePythonBackend.close()\n"                                                 \
                                     "del(shelve)\n"                                                                 \
                                     "del(shelvePythonBackend)\n"                                                    \
                                     "del(keyPythonBackend)\n";

    return classSavePythonSession.arg(fileName);
}

QString PythonVariableManagementExtension::loadVariables(const QString& fileName)
{
    QString classLoadPythonSession = "import shelve\n"                                                           \
                                     "shelvePythonBackend = shelve.open('%1')\n"                                 \
                                     "for keyPythonBackend in shelvePythonBackend:\n"                            \
                                     "    globals()[keyPythonBackend] = shelvePythonBackend[keyPythonBackend]\n" \
                                     "shelvePythonBackend.close()\n"                                             \
                                     "del(shelve)\n"                                                             \
                                     "del(shelvePythonBackend)\n"                                                \
                                     "del(keyPythonBackend)\n";

    return classLoadPythonSession.arg(fileName);
}
