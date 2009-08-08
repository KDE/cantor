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

#include "kdebug.h"

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

K_EXPORT_MATHEMATIK_PLUGIN(rbackend, RBackend)

#include "rbackend.moc"
