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

#include "maximabackend.h"

#include "maximasession.h"
#include "settings.h"
#include "ui_settings.h"
#include "maximaextensions.h"

#include "kdebug.h"
#include <QWidget>

#include "mathematik_macros.h"


MaximaBackend::MaximaBackend( QObject* parent,const QList<QVariant> args ) : MathematiK::Backend( parent,args )
{
    setObjectName("maximabackend");
    kDebug()<<"Creating MaximaBackend";
    //initialize the supported extensions
    new MaximaCASExtension(this);
    new MaximaCalculusExtension(this);
}

MaximaBackend::~MaximaBackend()
{
    kDebug()<<"Destroying MaximaBackend";
}

MathematiK::Session* MaximaBackend::createSession()
{
    kDebug()<<"Spawning a new Maxima session";

    return new MaximaSession(this);
}

MathematiK::Backend::Capabilities MaximaBackend::capabilities()
{
    kDebug()<<"Requesting capabilites of MaximaSession";
    return MathematiK::Backend::Nothing;
}

QWidget* MaximaBackend::settingsWidget(QWidget* parent)
{
    QWidget* widget=new QWidget(parent);
    Ui::MaximaSettingsBase s;
    s.setupUi(widget);
    return widget;
}

KConfigSkeleton* MaximaBackend::config()
{
    return MaximaSettings::self();
}

K_EXPORT_MATHEMATIK_PLUGIN(maximabackend, MaximaBackend)

#include "maximabackend.moc"
