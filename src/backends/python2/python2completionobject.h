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

#ifndef _PYTHON2COMPLETIONOBJECT_H
#define _PYTHON2COMPLETIONOBJECT_H

#include "completionobject.h"

class Python2Session;

class Python2CompletionObject : public Cantor::CompletionObject
{
  public:
    Python2CompletionObject(const QString& cmd, int index, Python2Session* session) ;
    ~Python2CompletionObject();

  protected:
    virtual bool mayIdentifierContain(QChar c) const;
    virtual bool mayIdentifierBeginWith(QChar c) const;

  protected Q_SLOTS:
    void fetchCompletions();
    void fetchIdentifierType();
};

#endif /* _PYTHON2COMPLETIONOBJECT_H */
