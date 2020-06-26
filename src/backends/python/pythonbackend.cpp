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
    Copyright (C) 2012 Filipe Saraiva <filipe@kde.org>
 */

#include "pythonbackend.h"
#include "pythonsession.h"
#include "pythonextensions.h"
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
    QWidget* widget=new QWidget(parent);
    Ui::PythonSettingsBase s;
    s.setupUi(widget);
    return widget;
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
    const QUrl& localDoc = PythonSettings::self()->localDoc();
    if (!localDoc.isEmpty())
        return localDoc;
    else
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
    const QString& path = PythonSettings::pythonServerPath().toLocalFile();
    return Cantor::Backend::checkExecutable(QLatin1String("Cantor Python Server"), path, reason);
}

K_PLUGIN_FACTORY_WITH_JSON(pythonbackend, "pythonbackend.json", registerPlugin<PythonBackend>();)
#include "pythonbackend.moc"

