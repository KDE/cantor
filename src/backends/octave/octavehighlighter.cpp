/*
    Copyright (C) 2010 Miha Čančula <miha.cancula@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.
*/

#include "octavehighlighter.h"
#include "result.h"
#include "session.h"

#include <QDebug>

#include <repository.h>
#include <KF5/KSyntaxHighlighting/Definition>

OctaveHighlighter::OctaveHighlighter(QObject* parent, Cantor::Session* session): DefaultHighlighter(parent), m_session(session)
{

  KSyntaxHighlighting::Repository m_repository;
  KSyntaxHighlighting::Definition definition = m_repository.definitionForName(QLatin1String("Octave"));

  //TODO: KSyntaxHighlighting provides "keywords", "functions", "forge", "builtin" and "commands".
  //we use "keywords" and "functions" at the moment. decide what to do with "forge", "builtin" and "commands".
  m_keywords = definition.keywordList(QLatin1String("keywords"));

  //TODO: the list of keywords in KSyntaxHighlighting doesn't have all the keywords added here.
  //add the missing keywords to KSyntaxHighlighting and remove the custom handling for them here later.
  m_keywords << QLatin1String("function") << QLatin1String("endfunction");
  m_keywords << QLatin1String("for") << QLatin1String("endfor");
  m_keywords << QLatin1String("while") << QLatin1String("endwhile");
  m_keywords << QLatin1String("if") << QLatin1String("endif") << QLatin1String("else") << QLatin1String("elseif");
  m_keywords << QLatin1String("switch") << QLatin1String("case") << QLatin1String("otherwise") << QLatin1String("endswitch");
  m_keywords << QLatin1String("end");
  addKeywords(m_keywords);

  m_functions = definition.keywordList(QLatin1String("functions"));
  //TODO: the list of keywords in KSyntaxHighlighting doesn't have all the functions for plotting that we take care of in the constructor of OctaveExpression.
  //add the missing functions to KSyntaxHighlighting and remove the custom handling for them here later.
  m_functions << QLatin1String("plot") << QLatin1String("semilogx") << QLatin1String("semilogy") << QLatin1String("loglog")
                   << QLatin1String("polar") << QLatin1String("contour") << QLatin1String("bar")
                   << QLatin1String("stairs") << QLatin1String("errorbar")  << QLatin1String("sombrero")
                   << QLatin1String("hist") << QLatin1String("fplot") << QLatin1String("imshow")
                   << QLatin1String("stem") << QLatin1String("stem3") << QLatin1String("scatter") << QLatin1String("pareto") << QLatin1String("rose")
                   << QLatin1String("pie") << QLatin1String("quiver") << QLatin1String("compass") << QLatin1String("feather")
                   << QLatin1String("pcolor") << QLatin1String("area") << QLatin1String("fill") << QLatin1String("comet")
                   << QLatin1String("plotmatrix")
                   /* 3d-plots */
                   << QLatin1String("plot3")
                   << QLatin1String("mesh") << QLatin1String("meshc") << QLatin1String("meshz")
                   << QLatin1String("surf") << QLatin1String("surfc") << QLatin1String("surfl") << QLatin1String("surfnorm")
                   << QLatin1String("isosurface")<< QLatin1String("isonormals") << QLatin1String("isocaps")
                   /* 3d-plots defined by a function */
                   << QLatin1String("ezplot3") << QLatin1String("ezmesh") << QLatin1String("ezmeshc") << QLatin1String("ezsurf") << QLatin1String("ezsurfc");

  addFunctions(m_functions);


  m_operators << QLatin1String("+") << QLatin1String("-") << QLatin1String("*") << QLatin1String("/") << QLatin1String(".+")
              << QLatin1String(".-") << QLatin1String(".*") << QLatin1String("./") << QLatin1String("=");
  m_operators << QLatin1String("or") << QLatin1String("and") << QLatin1String("xor") << QLatin1String("not");
  m_operators << QLatin1String("||") << QLatin1String("&&") << QLatin1String("==");
  addRules(m_operators, operatorFormat());

  addRule(QRegExp(QLatin1String("\"[^\"]*\"")), stringFormat());
  addRule(QRegExp(QLatin1String("'[^']*'")), stringFormat());

  rehighlight();
}

OctaveHighlighter::~OctaveHighlighter()
{

}

//TODO: old code to recieve the variables. This needs to be fixed as this has to be executed every time a new variable was defined
//and not only once in the constructor of OctaveHighlighter where the list of variables is empty at the beginning.

/*
void OctaveHighlighter::updateVariables()
{
  m_varsExpr = m_session->evaluateExpression(QLatin1String("who"));
  connect(m_varsExpr, &Cantor::Expression::statusChanged, this, &OctaveHighlighter::receiveVariables);
}

void OctaveHighlighter::receiveVariables()
{
  if (m_varsExpr->status() != Cantor::Expression::Done || !m_varsExpr->result())
  {
    return;
  }
  QString res = m_varsExpr->result()->toHtml();
  res.replace(QLatin1String("<br/>"),QLatin1String(" "));
  res.remove(0, res.indexOf(QLatin1Char('\n')));
  res.remove(QLatin1Char('\n'));
  res = res.trimmed();

  m_variables.clear();
  foreach ( const QString& var, res.split(QLatin1Char(' '), QString::SkipEmptyParts))
  {
      m_variables <<  var.trimmed();
  }
  qDebug() << "Received" << m_variables.size() << "variables";

  addVariables(m_variables);
  rehighlight();
}
*/
