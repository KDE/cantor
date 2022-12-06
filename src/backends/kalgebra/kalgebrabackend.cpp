/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kalgebrabackend.h"
#include "kalgebrasession.h"
#include "kalgebraextensions.h"
#include "settings.h"
#include "ui_settings.h"

#include <KPluginFactory>

KAlgebraBackend::KAlgebraBackend(QObject* parent,const QList<QVariant> args)
    : Cantor::Backend(parent, args)
{
    new KAlgebraVariableManagementExtension(this);
}

QString KAlgebraBackend::id() const
{
    return QLatin1String("kalgebra");
}

QString KAlgebraBackend::version() const
{
    return QLatin1String("Analitza version 5.0");
}

Cantor::Session* KAlgebraBackend::createSession()
{
    return new KAlgebraSession(this);
}

Cantor::Backend::Capabilities KAlgebraBackend::capabilities() const
{
    return Cantor::Backend::Completion | Cantor::Backend::SyntaxHighlighting
        | Cantor::Backend::SyntaxHelp | Cantor::Backend::VariableManagement;
}

QWidget* KAlgebraBackend::settingsWidget(QWidget* parent) const
{
    QWidget* widget = new QWidget(parent);
    Ui::KAlgebraSettingsBase s;
    s.setupUi(widget);
    return widget;
}

KConfigSkeleton* KAlgebraBackend::config() const
{
    return KAlgebraSettings::self();
}

QUrl KAlgebraBackend::helpUrl() const
{
    return QUrl(i18nc("The url to the documentation of KAlgebra, please check if there is a translated version and use the correct url",
                 "https://docs.kde.org/?application=kalgebra"));
}

bool KAlgebraBackend::requirementsFullfilled(QString* const reason) const
{
    Q_UNUSED(reason);
    return true;
}

K_PLUGIN_FACTORY_WITH_JSON(kalgebrabackend, "kalgebrabackend.json", registerPlugin<KAlgebraBackend>();)
#include "kalgebrabackend.moc"
