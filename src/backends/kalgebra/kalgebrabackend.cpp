/*************************************************************************************
*  Copyright (C) 2009 by Aleix Pol <aleixpol@kde.org>                               *
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

#include "kalgebrabackend.h"
#include "kalgebrasession.h"

#include "cantor_macros.h"

KAlgebraBackend::KAlgebraBackend( QObject* parent,const QList<QVariant> args )
    : Cantor::Backend( parent,args )
{
    setObjectName("kalgebrabackend");
}

KAlgebraBackend::~KAlgebraBackend()
{}

Cantor::Session* KAlgebraBackend::createSession()
{
    return new KAlgebraSession(this);
}

Cantor::Backend::Capabilities KAlgebraBackend::capabilities()
{
    return Cantor::Backend::TabCompletion | Cantor::Backend::SyntaxHighlighting | Cantor::Backend::SyntaxHelp;
}

KUrl KAlgebraBackend::helpUrl()
{
    return i18nc("the url to the documentation of KAlgebra, please check if there is a translated version and use the correct url",
                 "http://docs.kde.org/stable/en/kdeedu/kalgebra/");
}


K_EXPORT_CANTOR_PLUGIN(kalgebrabackend, KAlgebraBackend)
