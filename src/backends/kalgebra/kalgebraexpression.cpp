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

#include "kalgebraexpression.h"

#include "textresult.h"
#include "helpresult.h"
#include "kalgebrasession.h"
#include <KLocale>

#include <analitza/expression.h>
#include <analitza/analyzer.h>

KAlgebraExpression::KAlgebraExpression( KAlgebraSession* session )
    : Cantor::Expression(session)
{}

KAlgebraExpression::~KAlgebraExpression()
{}

void KAlgebraExpression::evaluate()
{
    setStatus(Cantor::Expression::Computing);

    Analitza::Analyzer* a=static_cast<KAlgebraSession*>(session())->analyzer();
    a->setExpression(Analitza::Expression(command()));

    Analitza::Expression res;
    if(a->isCorrect())
        res=a->evaluate();

    if(a->isCorrect()) {
        setResult(new Cantor::TextResult(res.toString()));
        setStatus(Cantor::Expression::Done);
    } else {
        QString error=i18n("Error: %1", a->errors().join("\n"));
        setErrorMessage(error);
        setStatus(Cantor::Expression::Error);
    }
}
