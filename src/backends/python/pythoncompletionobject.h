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
    Copyright (C) 2013 Filipe Saraiva <filipe@kde.org>
 */

#ifndef _PYTHONCOMPLETIONOBJECT_H
#define _PYTHONCOMPLETIONOBJECT_H

#include "completionobject.h"

class PythonSession;

class PythonCompletionObject : public Cantor::CompletionObject
{
  public:
    PythonCompletionObject(const QString& cmd, int index, PythonSession* session);
    ~PythonCompletionObject() override;

  protected:
    bool mayIdentifierContain(QChar c) const Q_DECL_OVERRIDE;
    bool mayIdentifierBeginWith(QChar c) const Q_DECL_OVERRIDE;

  protected Q_SLOTS:
    void fetchCompletions() Q_DECL_OVERRIDE;
    void fetchIdentifierType() Q_DECL_OVERRIDE;
};

#endif /* _PYTHONCOMPLETIONOBJECT_H */
