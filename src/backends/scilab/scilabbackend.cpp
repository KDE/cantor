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
    Copyright (C) 2011 Filipe Saraiva <filip.saraiva@gmail.com>
 */

#include "scilabbackend.h"

// #include "sagesession.h"
#include "settings.h"
#include "ui_settings.h"
// #include "sageextensions.h"
// #include "sagehighlighter.h"

#include "kdebug.h"
#include <QWidget>

#include "cantor_macros.h"


ScilabBackend::ScilabBackend( QObject* parent,const QList<QVariant> args ) : Cantor::Backend( parent,args )
{
    setObjectName("scilabbackend");
    kDebug()<<"Creating ScilabBackend";
    //initialize the supported extensions
//     new SageHistoryExtension(this);
//     new SageScriptExtension(this);
//     new SageCASExtension(this);
//     new SageCalculusExtension(this);
//     new SageLinearAlgebraExtension(this);
//     new SagePlotExtension(this);
}

ScilabBackend::~ScilabBackend()
{
    kDebug()<<"Destroying ScilabBackend";
}

QString ScilabBackend::id() const
{
    return "scilab";
}

// Cantor::Session* ScilabBackend::createSession()
// {
//     kDebug()<<"Spawning a new Sage session";
//
//     return new ScilabSession(this);
// }
//
// Cantor::Backend::Capabilities ScilabBackend::capabilities() const
// {
//     kDebug()<<"Requesting capabilities of SageSession";
//     return Cantor::Backend::LaTexOutput|Cantor::Backend::SyntaxHighlighting|Cantor::Backend::Completion;
// }
//
// bool ScilabBackend::requirementsFullfilled() const
// {
//     QFileInfo info(ScilabSettings::self()->path().toLocalFile());
//     return info.isExecutable();
// }

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

KUrl ScilabBackend::helpUrl() const
{
    return i18nc("the url to the documentation of Sage, please check if there is a translated version and use the correct url",
                 "http://www.sagemath.org/doc/reference/index.html");
}

QString ScilabBackend::description() const
{
    return i18n("Sage is a free open-source mathematics software system licensed under the GPL. <br/>" \
                "It combines the power of many existing open-source packages into a common Python-based interface.");
}

K_EXPORT_CANTOR_PLUGIN(sagebackend, ScilabBackend)

// #include "sagebackend.moc"
