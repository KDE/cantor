/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014 Lucas Hermann Negri <lucashnegri@gmail.com>
*/

#ifndef LUAEXTENSIONS_H
#define LUAEXTENSIONS_H

#include <extension.h>

class LuaScriptExtension : public Cantor::ScriptExtension
{
  public:
    explicit LuaScriptExtension(QObject* parent);
    ~LuaScriptExtension() override;

  public Q_SLOTS:
    QString scriptFileFilter() override;
    QString highlightingMode() override;
    QString runExternalScript(const QString& path) override;
    QString commandSeparator() override;
};

class LuaVariableManagementExtension : public Cantor::VariableManagementExtension
{
public:
    explicit LuaVariableManagementExtension(QObject* parent);
    ~LuaVariableManagementExtension() override;

    QString addVariable(const QString& name, const QString& value) override;
    QString setValue(const QString& name, const QString& value) override;
    QString removeVariable(const QString& name) override;
    QString saveVariables(const QString& fileName) override;
    QString loadVariables(const QString& fileName) override;
    QString clearVariables() override;
};

#endif // LUAEXTENSIONS_H
