/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2013 Filipe Saraiva <filipe@kde.org>
*/

#ifndef _IMPORTPACKAGEASSISTANT_H
#define _IMPORTPACKAGEASSISTANT_H

#include "assistant.h"

class ImportPackageAssistant : public Cantor::Assistant
{
  public:
    ImportPackageAssistant( QObject* parent, QList<QVariant> args );
    ~ImportPackageAssistant() override = default;

    void initActions() override;

    QStringList run(QWidget* parentt) override;

};

#endif /* _IMPORTPACKAGEASSISTANT_H */
