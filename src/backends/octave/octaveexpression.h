/*
    SPDX-FileCopyrightText: 2010 Miha Čančula <miha.cancula@gmail.com>
    SPDX-FileCopyrightText: 2018-2022 by Alexander Semke (alexander.semke@web.de)

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef OCTAVEEXPRESSION_H
#define OCTAVEEXPRESSION_H

#include <config-cantorlib.h>

#include <expression.h>
#include <QStringList>

class OctaveExpression : public Cantor::Expression
{
    Q_OBJECT

public:
    explicit OctaveExpression(Cantor::Session*, bool internal = false);
    ~OctaveExpression();

    void interrupt() override;
    void evaluate() override;
    QString internalCommand() override;

    void parseOutput(const QString&) override;
    void parseError(const QString&) override;
    void imageChanged();

    const static QStringList plotExtensions;

private:
    QString m_resultString;
    bool m_finished = false;
    bool m_plotPending = false;
    QString m_plotFilename;
};

#endif // OCTAVEEXPRESSION_H
