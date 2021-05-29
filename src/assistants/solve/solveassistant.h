/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _SOLVEASSISTANT_H
#define _SOLVEASSISTANT_H

#include "assistant.h"

class SolveAssistant : public Cantor::Assistant
{
  public:
    SolveAssistant( QObject* parent, QList<QVariant> args );
    ~SolveAssistant() override = default;

    void initActions() override;
    
    QStringList run(QWidget* parentt) override;
    
};

#endif /* _SOLVEASSISTANT_H */
