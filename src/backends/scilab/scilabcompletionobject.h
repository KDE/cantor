/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2011 Filipe Saraiva <filipe@kde.org>
*/

#ifndef _SCILABCOMPLETIONOBJECT_H
#define _SCILABCOMPLETIONOBJECT_H

#include "completionobject.h"

class ScilabSession;

class ScilabCompletionObject : public Cantor::CompletionObject
{
    public:
        ScilabCompletionObject(const QString& cmd, int index, ScilabSession* session) ;
        ~ScilabCompletionObject() override = default;

    protected:
        bool mayIdentifierContain(QChar c) const override;
        bool mayIdentifierBeginWith(QChar c) const override;

    protected Q_SLOTS:
        void fetchCompletions() override;
        void fetchIdentifierType() override;
};

#endif /* _SCILABCOMPLETIONOBJECT_H */
