/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _INVERTMATRIXASSISTANT_H
#define _INVERTMATRIXASSISTANT_H

#include "assistant.h"

class InvertMatrixAssistant : public Cantor::Assistant
{
  public:
    InvertMatrixAssistant( QObject* parent, QList<QVariant> args );
    ~InvertMatrixAssistant() override = default;

    void initActions() override;
    
    QStringList run(QWidget* parentt) override;
    
};

#endif /* _INVERTMATRIXASSISTANT_H */
