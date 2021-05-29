/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KALGEBRA_COMPLETIONOBJECT_H
#define KALGEBRA_COMPLETIONOBJECT_H

#include "completionobject.h"

class KAlgebraSession;

class KAlgebraCompletionObject : public Cantor::CompletionObject
{
    public:
         KAlgebraCompletionObject( const QString& command, int index, KAlgebraSession* session);
        ~KAlgebraCompletionObject() override = default;

    protected:
        bool mayIdentifierBeginWith(QChar c) const override;

    protected Q_SLOTS:
        void fetchCompletions() override;
};

#endif /* _NULLCOMPLETIONOBJECT_H */
