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
    Copyright (C) 2011 Filipe Saraiva <filipe@kde.org>
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
