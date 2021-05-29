/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2009 Oleksiy Protas <elfy.ua@gmail.com>
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
