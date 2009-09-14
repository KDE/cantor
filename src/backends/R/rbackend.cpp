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
#include "settings.h"
#include "ui_settings.h"

#include <kdebug.h>
#include <kstandarddirs.h>

#include "mathematik_macros.h"


RBackend::RBackend( QObject* parent,const QList<QVariant> args ) : MathematiK::Backend( parent,args )
{
    setObjectName("rbackend");
    kDebug()<<"Creating RBackend";
}

RBackend::~RBackend()
{
    kDebug()<<"Destroying RBackend";
}

MathematiK::Session* RBackend::createSession()
{
    kDebug()<<"Spawning a new R session";

    return new RSession(this);
}

MathematiK::Backend::Capabilities RBackend::capabilities()
{
    kDebug()<<"Requesting capabilities of RSession";
    return MathematiK::Backend::InteractiveMode;
}

bool RBackend::requirementsFullfilled()
{
    QFileInfo info(KStandardDirs::findExe( "mathematik_rserver" ) );
    return info.isExecutable();
}

QWidget* RBackend::settingsWidget(QWidget* parent)
{
    QWidget* widget=new QWidget(parent);
    Ui::RSettingsBase s;
    s.setupUi(widget);
    return widget;
}

KConfigSkeleton* RBackend::config()
{
    return RServerSettings::self();
}

QString RBackend::description()
{
    return i18n("R is a language and environment for statistical computing and graphics, similar to the S language and environment. <br/>"\
                "It provides a wide variety of statistical (linear and nonlinear modelling, "\
                "classical statistical tests, time-series analysis, classification, clustering, ...) "\
                "and graphical techniques, and is highly extensible. The S language is often the "\
                "vehicle of choice for research in statistical methodology, "\
                "and R provides an Open Source route to participation in that activity.");
}

K_EXPORT_MATHEMATIK_PLUGIN(rbackend, RBackend)

#include "rbackend.moc"
