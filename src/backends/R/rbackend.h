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

#ifndef _RBACKEND_H
#define _RBACKEND_H

#include "backend.h"

class RBackend : public MathematiK::Backend
{
  Q_OBJECT
  public:
    explicit RBackend( QObject* parent = 0,const QList<QVariant> args = QList<QVariant>());
    ~RBackend();

    MathematiK::Session *createSession();
    MathematiK::Backend::Capabilities capabilities();
    bool requirementsFullfilled();

    virtual QWidget* settingsWidget(QWidget* parent);
    virtual KConfigSkeleton* config();

};


#endif /* _RBACKEND_H */
