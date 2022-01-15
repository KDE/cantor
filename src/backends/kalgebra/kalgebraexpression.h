/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KALGEBRA_EXPRESSION_H
#define KALGEBRA_EXPRESSION_H

#include "expression.h"

class KAlgebraSession;

class KAlgebraExpression : public Cantor::Expression
{
    Q_OBJECT
    public:
        explicit KAlgebraExpression( KAlgebraSession* session, bool internal = false);
        ~KAlgebraExpression() override = default;

        void evaluate() override;
        void interrupt() override;

        void parseOutput(const QString&) override {};
        void parseError(const QString&) override {};
};

#endif
