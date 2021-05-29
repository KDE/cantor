/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KALGEBRA_SESSION_H
#define KALGEBRA_SESSION_H

#include "session.h"

class OperatorsModel;
class KAlgebraExpression;

namespace Analitza {
class Analyzer;
class VariablesModel;
}

class KAlgebraSession : public Cantor::Session
{
    Q_OBJECT
    public:
        explicit KAlgebraSession( Cantor::Backend* backend);
        ~KAlgebraSession() override;

        void login() override;
        void logout() override;

        void interrupt() override;

        Cantor::Expression* evaluateExpression(const QString& command, Cantor::Expression::FinishingBehavior behave = Cantor::Expression::FinishingBehavior::DoNotDelete, bool internal = false) override;
        Cantor::CompletionObject* completionFor(const QString& cmd, int index=-1) override;
        Cantor::SyntaxHelpObject* syntaxHelpFor(const QString& cmd) override;
        Analitza::Analyzer* analyzer() const { return m_analyzer; }
        OperatorsModel* operatorsModel();
        QSyntaxHighlighter* syntaxHighlighter(QObject* parent) override;
        QAbstractItemModel* variableDataModel() const override;

    private:
        Analitza::Analyzer* m_analyzer;
        OperatorsModel* m_operatorsModel;
        Analitza::VariablesModel* m_variablesModel;
};

#endif
