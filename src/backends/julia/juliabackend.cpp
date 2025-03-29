/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
    SPDX-FileCopyrightText: 2019-2023 Alexander Semke <alexander.semke@web.de>
*/

#include "juliabackend.h"
#include "juliaextensions.h"
#include "juliasession.h"
#include "juliasettingswidget.h"
#include "settings.h"

#include <KPluginFactory>

JuliaBackend::JuliaBackend(QObject *parent, const QList<QVariant> &args)
    : Cantor::Backend(parent, args)
{
    setEnabled(true);

    new JuliaVariableManagementExtension(this);
    new JuliaPackagingExtension(this);
    new JuliaPlotExtension(this);
    new JuliaScriptExtension(this);
    new JuliaLinearAlgebraExtension(this);
}

QString JuliaBackend::id() const
{
    return QLatin1String("julia");
}

QString JuliaBackend::version() const
{
    return QLatin1String("1.6.7");
}

Cantor::Session *JuliaBackend::createSession()
{
    return new JuliaSession(this);
}

Cantor::Backend::Capabilities JuliaBackend::capabilities() const
{
    static Cantor::Backend::Capabilities cap = SyntaxHighlighting | Completion | IntegratedPlots;

    if (JuliaSettings::variableManagement())
        return cap |= VariableManagement;
    else
        return cap;
}

QString JuliaBackend::description() const
{
    return i18n(
        "<b>Julia</b> is a high-level, high-performance dynamic programming "
        "language for technical computing, with syntax that is familiar to "
        "users of other technical computing environments. It provides a "
        "sophisticated compiler, distributed parallel execution, numerical "
        "accuracy, and an extensive mathematical function library."
    );
}

QUrl JuliaBackend::helpUrl() const
{
    return QUrl(i18nc(
        "The url to the documentation of Julia, please check if there is a"
        " translated version and use the correct url",
        "https://docs.julialang.org/en/latest/"
    ));
}

bool JuliaBackend::requirementsFullfilled(QString* const reason) const
{
#ifdef Q_OS_WIN
    const QString& path = QStandardPaths::findExecutable(QLatin1String("cantor_juliaserver.exe"));
#else
    const QString& path = QStandardPaths::findExecutable(QLatin1String("cantor_juliaserver"));
#endif
    return Cantor::Backend::checkExecutable(QLatin1String("Cantor Julia Server"), path, reason);
}

QWidget* JuliaBackend::settingsWidget(QWidget *parent) const
{
    return new JuliaSettingsWidget(parent, id());
}

KConfigSkeleton* JuliaBackend::config() const
{
    return JuliaSettings::self();
}

K_PLUGIN_FACTORY_WITH_JSON(
    juliabackend,
    "juliabackend.json",
    registerPlugin<JuliaBackend>();
)

#include "juliabackend.moc"
