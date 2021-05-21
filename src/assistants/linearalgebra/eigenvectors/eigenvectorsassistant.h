/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _EIGENVECTORSASSISTANT_H
#define _EIGENVECTORSASSISTANT_H

#include "assistant.h"

class EigenVectorsAssistant : public Cantor::Assistant
{
  public:
    EigenVectorsAssistant( QObject* parent, QList<QVariant> args );
    ~EigenVectorsAssistant() override = default;

    void initActions() override;
    
    QStringList run(QWidget* parentt) override;
    
};

#endif /* _EIGENVECTORSASSISTANT_H */
