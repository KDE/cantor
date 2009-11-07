/*************************************************************************************
*  Copyright (C) 2009 by Aleix Pol <aleixpol@kde.org>                               *
*                                                                                   *
*  This program is free software; you can redistribute it and/or                    *
*  modify it under the terms of the GNU General Public License                      *
*  as published by the Free Software Foundation; either version 2                   *
*  of the License, or (at your option) any later version.                           *
*                                                                                   *
*  This program is distributed in the hope that it will be useful,                  *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
*  GNU General Public License for more details.                                     *
*                                                                                   *
*  You should have received a copy of the GNU General Public License                *
*  along with this program; if not, write to the Free Software                      *
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
*************************************************************************************/

#ifndef KALGEBRA_SESSION_H
#define KALGEBRA_SESSION_H

#include "session.h"

class OperatorsModel;
class KAlgebraExpression;

namespace Analitza { class Analitza; }

class KAlgebraSession : public Cantor::Session
{
    Q_OBJECT
    public:
        KAlgebraSession( Cantor::Backend* backend);
        ~KAlgebraSession();

        void login();
        void logout();

        void interrupt();

        Cantor::Expression* evaluateExpression(const QString& command, Cantor::Expression::FinishingBehavior behave);
        Cantor::TabCompletionObject* tabCompletionFor(const QString& cmd);
        Cantor::SyntaxHelpObject* syntaxHelpFor(const QString& cmd);
        Analitza::Analitza* analitza() const { return m_analitza; }
        OperatorsModel* operatorsModel();
        QSyntaxHighlighter* syntaxHighlighter(QTextEdit* parent);

    private:
        Analitza::Analitza* m_analitza;
        OperatorsModel* m_operatorsModel;
};

#endif
