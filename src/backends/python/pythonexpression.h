/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Filipe Saraiva <filipe@kde.org>
*/

#ifndef _PYTHONEXPRESSION_H
#define _PYTHONEXPRESSION_H

#include "expression.h"

class PythonExpression : public Cantor::Expression
{
  Q_OBJECT
  public:
    PythonExpression(Cantor::Session*, bool internal);
    ~PythonExpression() override;

    void evaluate() override;
    void interrupt() override;
    QString internalCommand() override;

    void parseOutput(const QString&) override;
    void parseWarning(const QString&);
    void parseError(const QString&) override;
};

#endif /* _PYTHONEXPRESSION_H */
