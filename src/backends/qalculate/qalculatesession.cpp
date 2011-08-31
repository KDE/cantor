/************************************************************************************
*  Copyright (C) 2009 by Milian Wolff <mail@milianw.de>                             *
*  Copyright (C) 2011 by Matteo Agostinelli <agostinelli@gmail.com>                 *
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

#include "qalculatesession.h"
#include "qalculateexpression.h"
#include "qalculatecompletionobject.h"
#include "qalculatehighlighter.h"

#include <QTextEdit>

#include <libqalculate/Calculator.h>
#include <libqalculate/ExpressionItem.h>
#include <libqalculate/Unit.h>
#include <libqalculate/Prefix.h>
#include <libqalculate/Variable.h>
#include <libqalculate/Function.h>

#include <KDebug>

#include "qalculatesyntaxhelpobject.h"

QalculateSession::QalculateSession( Cantor::Backend* backend)
    : Session(backend)
{
    if ( !CALCULATOR ) {
             new Calculator();
             CALCULATOR->loadGlobalDefinitions();
             CALCULATOR->loadLocalDefinitions();
             CALCULATOR->loadExchangeRates();
    }
    // from qalc.cc in libqalculate
    std::string ansName = "ans";
    // m_undefined is not a variable in this class, but is defined in
    // libqalculate/includes.h
    m_ansVariables.append(static_cast<KnownVariable*>(CALCULATOR->addVariable(new KnownVariable("Temporary", ansName, m_undefined, "Last Answer", false))));
    m_ansVariables[0]->addName("answer");
    m_ansVariables[0]->addName(ansName + "1");
    m_ansVariables.append(static_cast<KnownVariable*>(CALCULATOR->addVariable(new KnownVariable("Temporary", ansName+"2", m_undefined, "Answer 2", false))));
    m_ansVariables.append(static_cast<KnownVariable*>(CALCULATOR->addVariable(new KnownVariable("Temporary", ansName+"3", m_undefined, "Answer 3", false))));
    m_ansVariables.append(static_cast<KnownVariable*>(CALCULATOR->addVariable(new KnownVariable("Temporary", ansName+"4", m_undefined, "Answer 4", false))));
    m_ansVariables.append(static_cast<KnownVariable*>(CALCULATOR->addVariable(new KnownVariable("Temporary", ansName+"5", m_undefined, "Answer 5", false))));
}

QalculateSession::~QalculateSession()
{
    CALCULATOR->abort();
}

void QalculateSession::login()
{
    changeStatus(Cantor::Session::Done);
    emit ready();
}

void QalculateSession::logout()
{
}

void QalculateSession::interrupt()
{
    changeStatus(Cantor::Session::Done);
}

Cantor::Expression* QalculateSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave)
{
    QalculateExpression* expr = new QalculateExpression(this);
    expr->setFinishingBehavior(behave);

    changeStatus(Cantor::Session::Running);
    expr->setCommand(cmd);
    expr->evaluate();
    changeStatus(Cantor::Session::Done);

    return expr;
}

Cantor::CompletionObject* QalculateSession::completionFor(const QString& command)
{
    return new QalculateCompletionObject(command, this);
}

Cantor::SyntaxHelpObject* QalculateSession::syntaxHelpFor(const QString& cmd)
{
    return new QalculateSyntaxHelpObject(cmd, this);
}

QSyntaxHighlighter* QalculateSession::syntaxHighlighter(QTextEdit* parent)
{
    return new QalculateHighlighter(parent);
}

void QalculateSession::setLastResult(MathStructure result)
{
    for (int i = m_ansVariables.size()-1; i >0 ; --i) {
	m_ansVariables[i]->set(m_ansVariables[i-1]->get());
    }
    m_ansVariables[0]->set(result);
}
