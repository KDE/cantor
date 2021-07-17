/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Filipe Saraiva <filipe@kde.org>
*/

#include "pythonbackend.h"
#include "pythonsession.h"
#include "pythonextensions.h"
#include "pythonsettingswidget.h"
#include "settings.h"
#include "ui_settings.h"

#include <QDebug>
#include <QWidget>

PythonBackend::PythonBackend(QObject* parent, const QList<QVariant>& args) : Cantor::Backend(parent, args)
{
    new PythonLinearAlgebraExtension(this);
    new PythonPackagingExtension(this);
    new PythonPlotExtension(this);
    new PythonScriptExtension(this);
    new PythonVariableManagementExtension(this);

    //setObjectName(QLatin1String("python3backend"));
}

QWidget* PythonBackend::settingsWidget(QWidget* parent) const
{
    return new PythonSettingsWidget(parent, id());
}

Cantor::Session* PythonBackend::createSession()
{
    return new PythonSession(this);
}

QString PythonBackend::id() const
{
    return QLatin1String("python");
}

QString PythonBackend::version() const
{
    return QLatin1String("3.6");
}

Cantor::Backend::Capabilities PythonBackend::capabilities() const
{
    qDebug()<<"Requesting capabilities of PythonSession";

    Backend::Capabilities cap =
        Cantor::Backend::SyntaxHighlighting |
        Cantor::Backend::Completion         |
        Cantor::Backend::SyntaxHelp         |
        Cantor::Backend::IntegratedPlots;

    if(PythonSettings::variableManagement())
        cap |= Cantor::Backend::VariableManagement;

    return cap;
}

QUrl PythonBackend::helpUrl() const
{
    return QUrl(i18nc("The url to the documentation Python", "https://docs.python.org/3/"));
}

QString PythonBackend::description() const
{
    return i18n("<b>Python</b> is a remarkably powerful dynamic programming language that is used in a wide variety of application domains. " \
                "There are several Python packages to scientific programming.");
}

KConfigSkeleton* PythonBackend::config() const
{
    return PythonSettings::self();
}

bool PythonBackend::requirementsFullfilled(QString* const reason) const
{
#ifdef Q_OS_WIN
    const QString& path = QStandardPaths::findExecutable(QLatin1String("cantor_pythonserver.exe"));
#else
    const QString& path = QStandardPaths::findExecutable(QLatin1String("cantor_pythonserver"));
#endif
    return Cantor::Backend::checkExecutable(QLatin1String("Cantor Python Server"), path, reason);
}

K_PLUGIN_FACTORY_WITH_JSON(pythonbackend, "pythonbackend.json", registerPlugin<PythonBackend>();)
#include "pythonbackend.moc"

