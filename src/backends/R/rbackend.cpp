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

#include "rbackend.h"

#include "rsession.h"
#include "rextensions.h"
#include "settings.h"
#include "ui_settings.h"

#include <kdebug.h>
#include <kstandarddirs.h>

#include "cantor_macros.h"


RBackend::RBackend( QObject* parent,const QList<QVariant> args ) : Cantor::Backend( parent,args )
{
    setObjectName("rbackend");
    kDebug()<<"Creating RBackend";

    new RScriptExtension(this);
}

RBackend::~RBackend()
{
    kDebug()<<"Destroying RBackend";
}

QString RBackend::id() const
{
    return "r";
}

Cantor::Session* RBackend::createSession()
{
    kDebug()<<"Spawning a new R session";

    return new RSession(this);
}

Cantor::Backend::Capabilities RBackend::capabilities() const
{
    kDebug()<<"Requesting capabilities of RSession";
    return  Cantor::Backend::InteractiveMode |
            Cantor::Backend::Completion;
}

bool RBackend::requirementsFullfilled() const
{
    QFileInfo info(KStandardDirs::findExe( "cantor_rserver" ) );
    return info.isExecutable();
}

QWidget* RBackend::settingsWidget(QWidget* parent) const
{
    QWidget* widget=new QWidget(parent);
    Ui::RSettingsBase s;
    s.setupUi(widget);
    return widget;
}

KConfigSkeleton* RBackend::config() const
{
    return RServerSettings::self();
}

KUrl RBackend::helpUrl() const
{
    return i18nc("the url to the documentation of R, please check if there is a translated version and use the correct url",
                 "http://rwiki.sciviews.org/doku.php?id=rdoc:rdoc" );
}

QString RBackend::description() const
{
    return i18n("R is a language and environment for statistical computing and graphics, similar to the S language and environment. <br/>"\
                "It provides a wide variety of statistical (linear and nonlinear modelling, "\
                "classical statistical tests, time-series analysis, classification, clustering, ...) "\
                "and graphical techniques, and is highly extensible. The S language is often the "\
                "vehicle of choice for research in statistical methodology, "\
                "and R provides an Open Source route to participation in that activity.");
}

K_EXPORT_CANTOR_PLUGIN(rbackend, RBackend)

#include "rbackend.moc"
