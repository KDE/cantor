/*
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2011 Matteo Agostinelli <agostinelli@gmail.com>
    SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qalculatebackend.h"
#include "settings.h" // settings.h must be included before qalculatesession.h
#include "qalculatesession.h"
#include "qalculateextensions.h"
#include "qalculatesettingswidget.h"

#include <KLocalizedString>
#include <KPluginFactory>

QalculateBackend::QalculateBackend(QObject* parent,const QList<QVariant> args)
  : Cantor::Backend(parent, args)
{
    new QalculateHistoryExtension(this);
    new QalculateVariableManagementExtension(this);
    new QalculateCalculusExtension(this);
    new QalculateCASExtension(this);
    new QalculateLinearAlgebraExtension(this);
    new QalculatePlotExtension(this);
}

QString QalculateBackend::id() const
{
    return QLatin1String("qalculate");
}

QString QalculateBackend::version() const
{
    return QLatin1String("4.8");
}

Cantor::Session* QalculateBackend::createSession()
{
    return new QalculateSession(this);
}

Cantor::Backend::Capabilities QalculateBackend::capabilities() const
{
    return Cantor::Backend::Completion | Cantor::Backend::SyntaxHighlighting | Cantor::Backend::SyntaxHelp | Cantor::Backend::VariableManagement;
}

QString QalculateBackend::description() const
{
    return i18n("<b>Qalculate!</b> is a multi-purpose cross-platform desktop calculator. "
    "It is simple to use but provides power and versatility normally reserved for complicated math packages, "
    "as well as useful tools for everyday needs (such as currency conversion and percent calculation). "
    "Features include a large library of customizable functions, unit calculations and conversion, "
    "physical constants, symbolic calculations (including integrals and equations), arbitrary precision, "
    "uncertainty propagation, interval arithmetic, plotting,");
}

QUrl QalculateBackend::helpUrl() const
{
    return QUrl(QString::fromLatin1("https://qalculate.github.io/manual/index.html"));
}

bool QalculateBackend::requirementsFullfilled(QString* const reason) const
{
    const QString& path = QalculateSettings::self()->path().toLocalFile();
    return Cantor::Backend::checkExecutable(QLatin1String("Qalculate!"), path, reason);
}

KConfigSkeleton* QalculateBackend::config() const
{
    return QalculateSettings::self();
}

QWidget* QalculateBackend::settingsWidget(QWidget* parent) const
{
    return new QalculateSettingsWidget(parent, id());
}

K_PLUGIN_FACTORY_WITH_JSON(qalculatebackend, "qalculatebackend.json", registerPlugin<QalculateBackend>();)
#include "qalculatebackend.moc"
