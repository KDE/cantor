/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _RUNSCRIPTASSISTANT_H
#define _RUNSCRIPTASSISTANT_H

#include "assistant.h"

class RunScriptAssistant : public Cantor::Assistant
{
  public:
    RunScriptAssistant( QObject* parent, QList<QVariant> args );
    ~RunScriptAssistant() override = default;

    void initActions() override;
    
    QStringList run(QWidget* parentt) override;
    
};

#endif /* _RUNSCRIPTASSISTANT_H */
