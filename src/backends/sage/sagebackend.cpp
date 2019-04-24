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


SageBackend::SageBackend( QObject* parent,const QList<QVariant>& args ) : Cantor::Backend( parent,args )
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

QString SageBackend::version() const
{
    return QLatin1String("8.1 and 8.2");
}

Cantor::Session* SageBackend::createSession()
{
    qDebug()<<"Spawning a new Sage session";

    return new SageSession(this);
}

Cantor::Backend::Capabilities SageBackend::capabilities() const
{
    qDebug()<<"Requesting capabilities of SageSession";
    //Disable Cantor::Backend::LaTexOutput, see sagesession.cpp:421
    return Cantor::Backend::SyntaxHighlighting|Cantor::Backend::Completion;
}

bool SageBackend::requirementsFullfilled(QString* const reason) const
{
    const QString& replPath = SageSettings::self()->path().toLocalFile();
    if (replPath.isEmpty())
    {
        if (reason)
            *reason = i18n("Sage backend needs installed Sage programming language. The backend often automatically founds needed Sage binary file, but not in this case. Please, go to Cantor settings and set path to Sage executable");
        return false;
    }

    QFileInfo info(replPath);
    if (info.isExecutable())
        return true;
    else
    {
        if (reason)
            *reason = i18n("In Sage backend settings a path to Sage binary file set as %1, but this file not executable. Do you sure, that this is correct path to Sage? Change this path in Cantor settings, if no.").arg(replPath);
        return false;
    }
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
