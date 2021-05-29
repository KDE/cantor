/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2013 Filipe Saraiva <filipe@kde.org>
*/

#ifndef SCILABEXTENSIONS_H
#define SCILABEXTENSIONS_H

#include <extension.h>

#define SCILAB_EXT_CDTOR_DECL(name) Scilab##name##Extension(QObject* parent); \
                                     ~Scilab##name##Extension();

class ScilabScriptExtension : public Cantor::ScriptExtension
{
    public:
        SCILAB_EXT_CDTOR_DECL(Script)
        QString scriptFileFilter() override;
        QString highlightingMode() override;
        QString runExternalScript(const QString& path) override;
        QString commandSeparator() override;
};

class ScilabVariableManagementExtension : public Cantor::VariableManagementExtension
{
    public:
        SCILAB_EXT_CDTOR_DECL(VariableManagement)
        QString addVariable(const QString& name, const QString& value) override;
        QString setValue(const QString& name, const QString& value) override;
        QString removeVariable(const QString& name) override;
        QString saveVariables(const QString& fileName) override;
        QString loadVariables(const QString& fileName) override;
        QString clearVariables() override;
};

#endif // SCILABEXTENSIONS_H
