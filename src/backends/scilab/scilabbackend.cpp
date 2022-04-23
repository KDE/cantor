/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2011 Filipe Saraiva <filipe@kde.org>
*/

#include "scilabbackend.h"
#include "scilabsession.h"
#include "scilabextensions.h"
#include "scilabsettingswidget.h"
#include "settings.h"

#include <KPluginFactory>

ScilabBackend::ScilabBackend(QObject* parent,const QList<QVariant> args) : Cantor::Backend(parent, args)
{
    new ScilabVariableManagementExtension(this);
    new ScilabScriptExtension(this);
}

ScilabBackend::~ScilabBackend()
{
    qDebug()<<"Destroying ScilabBackend";
}

QString ScilabBackend::id() const
{
    return QLatin1String("scilab");
}

QString ScilabBackend::version() const
{
    return QLatin1String("5.5, 6.0");
}

Cantor::Session* ScilabBackend::createSession()
{
    qDebug()<<"Spawning a new Scilab session";

    return new ScilabSession(this);
}

Cantor::Backend::Capabilities ScilabBackend::capabilities() const
{
    return Cantor::Backend::SyntaxHighlighting |
           Cantor::Backend::Completion         |
           Cantor::Backend::VariableManagement;
}

bool ScilabBackend::requirementsFullfilled(QString* const reason) const
{
    const QString& path = ScilabSettings::self()->path().toLocalFile();
    return Cantor::Backend::checkExecutable(QLatin1String("Scilab"), path, reason);
}

QWidget* ScilabBackend::settingsWidget(QWidget* parent) const
{
    return new ScilabSettingsWidget(parent, id());
}

KConfigSkeleton* ScilabBackend::config() const
{
    return ScilabSettings::self();
}

QUrl ScilabBackend::helpUrl() const
{
    return QUrl(i18nc("The url to the documentation of Scilab, please check if there is a translated version and use the correct url",
                      "https://www.scilab.org/support/documentation"));
}

QString ScilabBackend::description() const
{
    return i18n("<b>Scilab</b> is a free software, cross-platform numerical computational package and a high-level, numerically oriented programming language." \
        "Scilab is distributed under CeCILL license (GPL compatible).");
}

K_PLUGIN_FACTORY_WITH_JSON(scilabbackend, "scilabbackend.json", registerPlugin<ScilabBackend>();)
#include "scilabbackend.moc"
