/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _PLOT2DASSISTANT_H
#define _PLOT2DASSISTANT_H

#include "assistant.h"

class Plot2dAssistant : public Cantor::Assistant
{
  Q_OBJECT
  public:
    Plot2dAssistant( QObject* parent, QList<QVariant> args );
    ~Plot2dAssistant() override = default;

    void initActions() override;
    QStringList run(QWidget* parentt) override;
};

#endif /* _PLOT2DASSISTANT_H */
