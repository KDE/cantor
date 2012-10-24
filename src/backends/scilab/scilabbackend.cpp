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
#include "settings.h"
#include "ui_settings.h"

#include "kdebug.h"
#include <QWidget>

#include "cantor_macros.h"

ScilabBackend::ScilabBackend( QObject* parent,const QList<QVariant> args ) : Cantor::Backend( parent,args )
{
    kDebug()<<"Creating ScilabBackend";

    setObjectName("scilabbackend");
}

ScilabBackend::~ScilabBackend()
{
    kDebug()<<"Destroying ScilabBackend";
}

QString ScilabBackend::id() const
{
    return "scilab";
}

Cantor::Session* ScilabBackend::createSession()
{
    kDebug()<<"Spawning a new Scilab session";

    return new ScilabSession(this);
}

Cantor::Backend::Capabilities ScilabBackend::capabilities() const
{
    kDebug()<<"Requesting capabilities of ScilabSession";

    return Cantor::Backend::SyntaxHighlighting |
           Cantor::Backend::Completion;
}

bool ScilabBackend::requirementsFullfilled() const
{
    QFileInfo info(ScilabSettings::self()->path().toLocalFile());
    return info.isExecutable();
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

KUrl ScilabBackend::helpUrl() const
{
    return i18nc("the url to the documentation of Scilab, please check if there is a translated version and use the correct url", "http://www.scilab.org/support/documentation");
}

QString ScilabBackend::description() const
{
    return i18n("<p><b>Warning:</b> this backend works only with Scilab version 5.4 or later</p>"\
        "<p>Scilab is a free software, cross-platform numerical computational package and a high-level, numerically oriented programming language.</p>" \
        "Scilab is distributed under CeCILL license (GPL compatible)");
}

K_EXPORT_CANTOR_PLUGIN(scilabbackend, ScilabBackend)

#include "scilabbackend.moc"
