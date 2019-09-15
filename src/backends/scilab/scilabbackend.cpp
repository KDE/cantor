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
    Copyright (C) 2011 Filipe Saraiva <filipe@kde.org>
 */

#include "scilabbackend.h"
#include "scilabsession.h"
#include "scilabextensions.h"
#include "settings.h"
#include "ui_settings.h"


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
    QWidget* widget=new QWidget(parent);
    Ui::ScilabSettingsBase s;
    s.setupUi(widget);
    return widget;
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
