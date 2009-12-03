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
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
 */

#ifndef _MAXIMABACKEND_H
#define _MAXIMABACKEND_H

#include "backend.h"

class MaximaBackend : public Cantor::Backend
{
  Q_OBJECT
  public:
    explicit MaximaBackend( QObject* parent = 0,const QList<QVariant> args = QList<QVariant>());
    ~MaximaBackend();

    Cantor::Session *createSession();
    Cantor::Backend::Capabilities capabilities() const;
    bool requirementsFullfilled() const;

    KUrl helpUrl() const;
    QWidget* settingsWidget(QWidget* parent) const;
    KConfigSkeleton* config() const;

    QString description() const;
};


#endif /* _MAXIMABACKEND_H */
