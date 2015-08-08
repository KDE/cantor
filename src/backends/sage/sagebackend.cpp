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
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
 */

#include "sagebackend.h"

#include "sagesession.h"
#include "settings.h"
#include "ui_settings.h"
#include "sageextensions.h"
#include "sagehighlighter.h"

#include <QDebug>
#include <QWidget>

#include "cantor_macros.h"


SageBackend::SageBackend( QObject* parent,const QList<QVariant> args ) : Cantor::Backend( parent,args )
{
    setObjectName(QLatin1String("sagebackend"));
    qDebug()<<"Creating SageBackend";
    //initialize the supported extensions
    new SageHistoryExtension(this);
    new SageScriptExtension(this);
    new SageCASExtension(this);
    new SageCalculusExtension(this);
    new SageLinearAlgebraExtension(this);
    new SagePlotExtension(this);
    new SagePackagingExtension(this);
}

SageBackend::~SageBackend()
{
    qDebug()<<"Destroying SageBackend";
}

QString SageBackend::id() const
{
    return QLatin1String("sage");
}

Cantor::Session* SageBackend::createSession()
{
    qDebug()<<"Spawning a new Sage session";

    return new SageSession(this);
}

Cantor::Backend::Capabilities SageBackend::capabilities() const
{
    qDebug()<<"Requesting capabilities of SageSession";
    return Cantor::Backend::LaTexOutput|Cantor::Backend::SyntaxHighlighting|Cantor::Backend::Completion;
}

bool SageBackend::requirementsFullfilled() const
{
    QFileInfo info(SageSettings::self()->path().toLocalFile());
    return info.isExecutable();
}

QWidget* SageBackend::settingsWidget(QWidget* parent) const
{
    QWidget* widget=new QWidget(parent);
    Ui::SageSettingsBase s;
    s.setupUi(widget);
    return widget;
}

KConfigSkeleton* SageBackend::config() const
{
    return SageSettings::self();
}

QUrl SageBackend::helpUrl() const
{
    return QUrl(i18nc("the url to the documentation of Sage, please check if there is a translated version and use the correct url",
                 "http://www.sagemath.org/doc/reference/index.html"));
}

QString SageBackend::description() const
{
    return i18n("Sage is a free open-source mathematics software system licensed under the GPL. <br/>" \
                "It combines the power of many existing open-source packages into a common Python-based interface.");
}

K_PLUGIN_FACTORY_WITH_JSON(sagebackend, "sagebackend.json", registerPlugin<SageBackend>();)
#include "sagebackend.moc"
