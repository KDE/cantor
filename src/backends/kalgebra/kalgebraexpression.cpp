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
#include <qmlresult.h>
#include "kalgebrasession.h"
#include <KLocale>

#include <analitza/expression.h>
#include <analitza/expressionstream.h>
#include <analitza/analyzer.h>
#include <analitza/variables.h>
#include <analitzaplot/plotsmodel.h>
#include <analitzaplot/plotsfactory.h>
#include <analitzaplot/planecurve.h>
#include <analitzagui/variablesmodel.h>


KAlgebraExpression::KAlgebraExpression( KAlgebraSession* session )
    : Cantor::Expression(session)
    , m_session(session)
{
    m_model = new PlotsModel(this);
    m_model->setResolution(400);
}

KAlgebraExpression::~KAlgebraExpression()
{}

void KAlgebraExpression::evaluate()
{
    setStatus(Cantor::Expression::Computing);

//     Analitza::Analyzer* a=static_cast<KAlgebraSession*>(session())->analyzer();
//     Analitza::Expression res;
//     QString cmd = command();
//     QTextStream stream(&cmd);
//
//     Analitza::ExpressionStream s(&stream);
//     for(; !s.atEnd();) {
//         a->setExpression(s.next());
//         res = a->evaluate();
//
//         if(!a->isCorrect())
//             break;
//     }

//     if(a->isCorrect()) {
    addFunction(command(), -10, 10);
    QVariantMap vals;
    vals.insert("funcsModel", qVariantFromValue<QObject*>(m_model));
    setResult(new Cantor::QmlResult(
        "import QtQuick 1.1\n"
        "import org.kde.analitza 1.0\n"
        "Graph2DView { \n"
        "    width: 300; height:300\n"
        "    model: funcsModel \n"
        "}", vals)
    );
//         setStatus(Cantor::Expression::Done);
//     } else {
//         setErrorMessage(i18n("Error: %1", a->errors().join("\n")));
//         setStatus(Cantor::Expression::Error);
//     }
}

void KAlgebraExpression::interrupt()
{}


QStringList KAlgebraExpression::addFunction(const QString& expression, double up, double down)
{
    Analitza::Variables* vars = m_session->variables();
    Analitza::Expression e(expression, Analitza::Expression::isMathML(expression));
    QString fname;
    do {
        fname = m_model->freeId();
    } while(vars->contains(fname));
    QColor fcolor = Qt::red;

    QStringList err;
    PlotBuilder req = PlotsFactory::self()->requestPlot(e, Dim2D);
    PlaneCurve* it = static_cast<PlaneCurve*>(req.create(fcolor, fname, vars));
    if(up!=down)
        it->setInterval(it->parameters().first(), down, up);

    if(it->isCorrect())
        m_model->addPlot(it);
    else {
        err = it->errors();
        qDebug() << "loooooool" << err;
        delete it;
    }

    return err;
}