/*************************************************************************************
*  Copyright (C) 2009 by Aleix Pol <aleixpol@kde.org>                               *
*                                                                                   *
*  This program is free software; you can redistribute it and/or                    *
*  modify it under the terms of the GNU General Public License                      *
*  as published by the Free Software Foundation; either version 2                   *
*  of the License, or (at your option) any later version.                           *
*                                                                                   *
*  This program is distributed in the hope that it will be useful,                  *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
*  GNU General Public License for more details.                                     *
*                                                                                   *
*  You should have received a copy of the GNU General Public License                *
*  along with this program; if not, write to the Free Software                      *
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
*************************************************************************************/

#include "kalgebrabackend.h"
#include "kalgebrasession.h"
#include "kalgebraextensions.h"
#include "settings.h"
#include "ui_settings.h"

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
                 "http://docs.kde.org/stable/en/kdeedu/kalgebra/"));
}

bool KAlgebraBackend::requirementsFullfilled(QString* const reason) const
{
    Q_UNUSED(reason);
    return true;
}

K_PLUGIN_FACTORY_WITH_JSON(kalgebrabackend, "kalgebrabackend.json", registerPlugin<KAlgebraBackend>();)
#include "kalgebrabackend.moc"
