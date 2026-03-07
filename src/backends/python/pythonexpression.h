/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Filipe Saraiva <filipe@kde.org>
*/

#ifndef _PYTHONEXPRESSION_H
#define _PYTHONEXPRESSION_H

#include "expression.h"
class QTemporaryFile;

class PythonExpression : public Cantor::Expression
{
  Q_OBJECT
  public:
    PythonExpression(Cantor::Session*, bool internal);
    ~PythonExpression() override;

    void evaluate() override;
    QString internalCommand() override;

    void parseOutput(const QString&) override;
    void parseWarning(const QString&);

private:
    void imageChanged();
    QTemporaryFile* m_tempFile{nullptr};
};

#endif /* _PYTHONEXPRESSION_H */
