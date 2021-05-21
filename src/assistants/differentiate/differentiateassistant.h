/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _DIFFERENTIATEASSISTANT_H
#define _DIFFERENTIATEASSISTANT_H

#include "assistant.h"

class DifferentiateAssistant : public Cantor::Assistant
{
  public:
    DifferentiateAssistant( QObject* parent, QList<QVariant> args );
    ~DifferentiateAssistant() override = default;

    void initActions() override;
    
    QStringList run(QWidget* parentt) override;
    
};

#endif /* _DIFFERENTIATEASSISTANT_H */
