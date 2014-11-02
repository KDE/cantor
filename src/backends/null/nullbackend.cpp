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

#include "nullbackend.h"

#include "nullsession.h"

#include <QDebug>

#include "cantor_macros.h"


NullBackend::NullBackend( QObject* parent,const QList<QVariant> args ) : Cantor::Backend( parent,args )
{
    setObjectName(QLatin1String("nullbackend"));
    qDebug()<<"Creating NullBackend";
    setEnabled(false);
}

NullBackend::~NullBackend()
{
    qDebug()<<"Destroying NullBackend";
}

QString NullBackend::id() const
{
    return QLatin1String("null");
}

Cantor::Session* NullBackend::createSession()
{
    qDebug()<<"Spawning a new Null session";

    return new NullSession(this);
}

Cantor::Backend::Capabilities NullBackend::capabilities() const
{
    qDebug()<<"Requesting capabilities of NullSession";
    return Cantor::Backend::Nothing;
}

K_EXPORT_CANTOR_PLUGIN(nullbackend, NullBackend)

#include "nullbackend.moc"
