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

#ifndef KALGEBRA_COMPLETIONOBJECT_H
#define KALGEBRA_COMPLETIONOBJECT_H

#include "completionobject.h"

class KAlgebraSession;

class KAlgebraCompletionObject : public Cantor::CompletionObject
{
    public:
         KAlgebraCompletionObject( const QString& command, int index, KAlgebraSession* session);
        ~KAlgebraCompletionObject() override;

    protected:
        bool mayIdentifierBeginWith(QChar c) const override;

    protected Q_SLOTS:
        void fetchCompletions() override;
};

#endif /* _NULLCOMPLETIONOBJECT_H */
