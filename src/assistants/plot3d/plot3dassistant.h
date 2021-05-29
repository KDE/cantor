/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
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
