/*************************************************************************************
*  Copyright (C) 2009 by Milian Wolff <mail@milianw.de>                               *
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

#ifndef QALCULATE_SESSION_H
#define QALCULATE_SESSION_H

#include "session.h"

#include <QSharedPointer>

#include <libqalculate/Variable.h>
#include <libqalculate/MathStructure.h>

namespace Cantor {
class DefaultVariableModel;
}

class QalculateEngine;

class QalculateSession : public Cantor::Session
{
    Q_OBJECT

private:
    QList<KnownVariable*> m_ansVariables;
    Cantor::DefaultVariableModel* m_variableModel;

public:
    QalculateSession( Cantor::Backend* backend);
    ~QalculateSession();

    virtual void login();
    virtual void logout();

    virtual void interrupt();

    virtual Cantor::Expression* evaluateExpression(const QString& command, Cantor::Expression::FinishingBehavior behave);
    virtual Cantor::CompletionObject* completionFor(const QString& cmd, int index=-1);
    virtual Cantor::SyntaxHelpObject* syntaxHelpFor(const QString& cmd);
    virtual QSyntaxHighlighter* syntaxHighlighter(QObject* parent);

    void setLastResult(MathStructure);
    QAbstractItemModel* variableModel();
};

#endif
