/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _INTEGRATEASSISTANT_H
#define _INTEGRATEASSISTANT_H

#include "assistant.h"

class IntegrateAssistant : public Cantor::Assistant
{
  public:
    IntegrateAssistant( QObject* parent, QList<QVariant> args );
    ~IntegrateAssistant() override = default;

    void initActions() override;
    
    QStringList run(QWidget* parentt) override;
    
};

#endif /* _INTEGRATEASSISTANT_H */
