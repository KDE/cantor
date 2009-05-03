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

#include "kdebug.h"

#include "mathematik_macros.h"


NullBackend::NullBackend( QObject* parent,const QList<QVariant> args ) : MathematiK::Backend( parent,args )
{
    setObjectName("nullbackend");
    kDebug()<<"Creating NullBackend";
}

NullBackend::~NullBackend()
{
    kDebug()<<"Destroying NullBackend";
}

MathematiK::Session* NullBackend::createSession()
{
    kDebug()<<"Spawning a new Null session";

    return new NullSession(this);
}

MathematiK::Backend::Capabilities NullBackend::capabilities()
{
    kDebug()<<"Requesting capabilites of NullSession";
    return MathematiK::Backend::Nothing;
}

K_EXPORT_MATHEMATIK_PLUGIN(nullbackend, NullBackend)

#include "nullbackend.moc"
