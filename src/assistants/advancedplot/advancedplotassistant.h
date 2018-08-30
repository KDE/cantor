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
    Copyright (C) 2009 Oleksiy Protas <elfy.ua@gmail.com>
 */

#ifndef _ADVANCED_PLOTASSISTANT_H
#define _ADVANCED_PLOTASSISTANT_H

#include "assistant.h"

// WARNING: this assistant serves as a stub to implement the rich plotting features of R.
// therefore, when finished it is to be backported and merged with plot2d

class AdvancedPlotAssistant : public Cantor::Assistant
{
  Q_OBJECT
  public:
    AdvancedPlotAssistant( QObject* parent, QList<QVariant> args );
    ~AdvancedPlotAssistant() override = default;

    void initActions() override;

    QStringList run(QWidget* parentt) override;

};

#endif /* _RPLOTASSISTANT_H */
