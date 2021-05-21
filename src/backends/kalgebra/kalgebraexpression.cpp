/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kalgebraexpression.h"

#include "textresult.h"
#include "helpresult.h"
#include "kalgebrasession.h"
#include <KLocalizedString>

#include <analitza/expression.h>
#include <analitza/expressionstream.h>
#include <analitza/analyzer.h>

KAlgebraExpression::KAlgebraExpression( KAlgebraSession* session, bool internal)
    : Cantor::Expression(session, internal)
{}

void KAlgebraExpression::evaluate()
{
    setStatus(Cantor::Expression::Computing);

    Analitza::Analyzer* a=static_cast<KAlgebraSession*>(session())->analyzer();
    Analitza::Expression res;
    QString cmd = command();
    QTextStream stream(&cmd);

    Analitza::ExpressionStream s(&stream);
    for(; !s.atEnd();) {
        a->setExpression(s.next());
        res = a->evaluate();

        if(!a->isCorrect())
            break;
    }

    if(a->isCorrect()) {
        setResult(new Cantor::TextResult(res.toString()));
        setStatus(Cantor::Expression::Done);
    } else {
        setErrorMessage(i18n("Error: %1", a->errors().join(QLatin1String("\n"))));
        setStatus(Cantor::Expression::Error);
    }
}

void KAlgebraExpression::interrupt()
{}
