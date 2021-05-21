/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KALGEBRASYNTAXHELPOBJECT_H
#define KALGEBRASYNTAXHELPOBJECT_H

#include <syntaxhelpobject.h>

class KAlgebraSession;

class KAlgebraSyntaxHelpObject : public Cantor::SyntaxHelpObject
{
    public:
         KAlgebraSyntaxHelpObject( const QString& command, KAlgebraSession* session );
    
    protected:
        void fetchInformation() override;
};

#endif // KALGEBRASYNTAXHELPOBJECT_H
