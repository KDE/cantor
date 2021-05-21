/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _CREATEMATRIXASSISTANT_H
#define _CREATEMATRIXASSISTANT_H

#include "assistant.h"

class CreateMatrixAssistant : public Cantor::Assistant
{
  public:
    CreateMatrixAssistant( QObject* parent, QList<QVariant> args );
    ~CreateMatrixAssistant() override = default;

    void initActions() override;
    
    QStringList run(QWidget* parentt) override;
    
};

#endif /* _CREATEMATRIXASSISTANT_H */
