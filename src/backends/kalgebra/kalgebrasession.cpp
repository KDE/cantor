/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kalgebrasession.h"

#include "settings.h"

#include "kalgebraexpression.h"
#include "kalgebracompletionobject.h"
#include <analitzagui/algebrahighlighter.h>
#include <analitza/analyzer.h>
#include <QTextEdit>

#include <QDebug>
#include "kalgebrasyntaxhelpobject.h"
#include <analitzagui/operatorsmodel.h>
#include <analitzagui/variablesmodel.h>

KAlgebraSession::KAlgebraSession( Cantor::Backend* backend)
    : Session(backend)
{
    m_analyzer = new Analitza::Analyzer;
    m_operatorsModel = new OperatorsModel;
    m_variablesModel = new Analitza::VariablesModel(m_analyzer->variables());
    m_operatorsModel->setVariables(m_analyzer->variables());
}

KAlgebraSession::~KAlgebraSession()
{
    delete m_analyzer;
}

void KAlgebraSession::login()
{
    Q_EMIT loginStarted();
    if(!KAlgebraSettings::autorunScripts().isEmpty()){
        QString autorunScripts = KAlgebraSettings::self()->autorunScripts().join(QLatin1String("\n"));

        evaluateExpression(autorunScripts, KAlgebraExpression::DeleteOnFinish, true);
    }

    changeStatus(Cantor::Session::Done);
    Q_EMIT loginDone();
}

void KAlgebraSession::logout()
{
    Session::logout();
}

void KAlgebraSession::interrupt()
{
    changeStatus(Cantor::Session::Done);
}

Cantor::Expression* KAlgebraSession::evaluateExpression(const QString& cmd,
                                                        Cantor::Expression::FinishingBehavior behave,
                                                        bool internal)
{
    KAlgebraExpression* expr=new KAlgebraExpression(this, internal);
    expr->setFinishingBehavior(behave);

    changeStatus(Cantor::Session::Running);
    expr->setCommand(cmd);
    expr->evaluate();
    changeStatus(Cantor::Session::Done);

    m_operatorsModel->setVariables(m_analyzer->variables());
    m_variablesModel->updateInformation();
    return expr;
}

Cantor::CompletionObject* KAlgebraSession::completionFor(const QString& command, int index)
{
    return new KAlgebraCompletionObject(command, index, this);
}

Cantor::SyntaxHelpObject* KAlgebraSession::syntaxHelpFor(const QString& cmd)
{
    return new KAlgebraSyntaxHelpObject(cmd, this);
}

OperatorsModel* KAlgebraSession::operatorsModel()
{
    return m_operatorsModel;
}

QSyntaxHighlighter* KAlgebraSession::syntaxHighlighter(QObject* parent)
{
    Q_UNUSED(parent);
    //return new AlgebraHighlighter(parent->document());
    // TODO: Think of something better here.
    return new AlgebraHighlighter(nullptr);
}

QAbstractItemModel* KAlgebraSession::variableDataModel() const
{
    return m_variablesModel;
}
