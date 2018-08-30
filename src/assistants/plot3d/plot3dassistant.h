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

#ifndef _PLOT3DASSISTANT_H
#define _PLOT3DASSISTANT_H

#include "assistant.h"

class Plot3dAssistant : public Cantor::Assistant
{
  Q_OBJECT
  public:
    Plot3dAssistant( QObject* parent, QList<QVariant> args );
    ~Plot3dAssistant() override = default;

    void initActions() override;
    QStringList run(QWidget* parentt) override;
};

#endif /* _PLOT3DASSISTANT_H */
