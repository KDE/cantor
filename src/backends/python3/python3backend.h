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
    Copyright (C) 2014, 2015 Minh Ngo <minh@fedoraproject.org>
 */

#ifndef _PYTHON3BACKEND_H
#define _PYTHON3BACKEND_H

#include "../python/pythonbackend.h"

class Python3Backend : public PythonBackend
{
  public:
    explicit Python3Backend(QObject* parent = nullptr, const QList<QVariant> args = QList<QVariant>());

    Cantor::Session* createSession();

    QString id() const;
    KUrl helpUrl() const;
    QString description() const;
   
    KConfigSkeleton* config() const;
};

#endif
