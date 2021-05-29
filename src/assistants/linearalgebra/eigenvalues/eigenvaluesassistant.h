/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _EIGENVALUESASSISTANT_H
#define _EIGENVALUESASSISTANT_H

#include "assistant.h"

class EigenValuesAssistant : public Cantor::Assistant
{
  public:
    EigenValuesAssistant( QObject* parent, QList<QVariant> args );
    ~EigenValuesAssistant() override = default;

    void initActions() override;
    
    QStringList run(QWidget* parentt) override;
    
};

#endif /* _EIGENVALUESASSISTANT_H */
