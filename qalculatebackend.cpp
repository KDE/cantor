/*************************************************************************************
*  Copyright (C) 2009 by Milian Wolff <mail@milianw.de>                               *
*                                                                                   *
*  This program is free software; you can redistribute it and/or                    *
*  modify it under the terms of the GNU General Public License                      *
*  as published by the Free Software Foundation; either version 2                   *
*  of the License, or (at your option) any later version.                           *
*                                                                                   *
*  This program is distributed in the hope that it will be useful,                  *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
*  GNU General Public License for more details.                                     *
*                                                                                   *
*  You should have received a copy of the GNU General Public License                *
*  along with this program; if not, write to the Free Software                      *
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
*************************************************************************************/

#include "qalculatebackend.h"
#include "qalculatesession.h"

#include "cantor_macros.h"

#include <libqalculate/Calculator.h>

QalculateBackend::QalculateBackend( QObject* parent,const QList<QVariant> args )
  : Cantor::Backend( parent, args )
{
    setObjectName("qalculatebackend");

    if ( !CALCULATOR ) {
        new Calculator();
        CALCULATOR->loadGlobalDefinitions();
        CALCULATOR->loadLocalDefinitions();
        CALCULATOR->loadExchangeRates();
    }
}

QalculateBackend::~QalculateBackend()
{
    CALCULATOR->abort();
}

Cantor::Session* QalculateBackend::createSession()
{
    return new QalculateSession(this);
}

Cantor::Backend::Capabilities QalculateBackend::capabilities() const
{
    return Cantor::Backend::TabCompletion;
//     return Cantor::Backend::TabCompletion | Cantor::Backend::SyntaxHelp;
}

KUrl QalculateBackend::helpUrl() const
{
    //TODO: not really a "helpUrl", is it?
    return KUrl("http://qalculate.sourceforge.net/");
}

K_EXPORT_CANTOR_PLUGIN(qalculatebackend, QalculateBackend)
