/*
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QALCULATE_COMPLETIONOBJECT_H
#define QALCULATE_COMPLETIONOBJECT_H

#include "completionobject.h"

class QalculateSession;

class QalculateCompletionObject : public Cantor::CompletionObject
{
    public:
        QalculateCompletionObject( const QString& command, int index, QalculateSession* session);
        ~QalculateCompletionObject() override = default;

    protected:
        int locateIdentifier(const QString& cmd, int index) const override;

    protected Q_SLOTS:
        void fetchCompletions() override;
        void fetchIdentifierType() override;
};

#endif /* _NULLCOMPLETIONOBJECT_H */
