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

OctaveHighlighter::OctaveHighlighter(QObject* parent, Cantor::Session* session): DefaultHighlighter(parent), m_session(session)
{
  updateFunctions();
  updateVariables();

  m_operators << QLatin1String("+") << QLatin1String("-") << QLatin1String("*") << QLatin1String("/") << QLatin1String(".+")
              << QLatin1String(".-") << QLatin1String(".*") << QLatin1String("./") << QLatin1String("=");
  m_operators << QLatin1String("or") << QLatin1String("and") << QLatin1String("xor") << QLatin1String("not");
  m_operators << QLatin1String("||") << QLatin1String("&&") << QLatin1String("==");
  addRules(m_operators, operatorFormat());

  m_keywords << QLatin1String("function") << QLatin1String("endfunction");
  m_keywords << QLatin1String("for") << QLatin1String("endfor");
  m_keywords << QLatin1String("while") << QLatin1String("endwhile");
  m_keywords << QLatin1String("if") << QLatin1String("endif") << QLatin1String("else") << QLatin1String("elseif");
  m_keywords << QLatin1String("switch") << QLatin1String("case") << QLatin1String("otherwise") << QLatin1String("endswitch");
  m_keywords << QLatin1String("end");
  addKeywords(m_keywords);

  addRule(QRegExp(QLatin1String("\"[^\"]*\"")), stringFormat());
  addRule(QRegExp(QLatin1String("'[^']*'")), stringFormat());

  rehighlight();
}

OctaveHighlighter::~OctaveHighlighter()
{

}

void OctaveHighlighter::updateFunctions()
{
  m_functionsExpr = m_session->evaluateExpression(QLatin1String("completion_matches('')"));
  connect(m_functionsExpr, &Cantor::Expression::statusChanged, this, &OctaveHighlighter::receiveFunctions);
}

void OctaveHighlighter::updateVariables()
{
  m_varsExpr = m_session->evaluateExpression(QLatin1String("who"));
  connect(m_varsExpr, &Cantor::Expression::statusChanged, this, &OctaveHighlighter::receiveVariables);
}


void OctaveHighlighter::receiveFunctions()
{
  qDebug();
  if (m_functionsExpr->status() != Cantor::Expression::Done || !m_functionsExpr->result())
  {
    return;
  }

  QStringList names = m_functionsExpr->result()->toHtml().split(QLatin1String("<br/>\n"));

  QLatin1String under("__");
  while (!names.first().contains(under))
  {
    names.removeFirst();
  }
  while (names.first().contains(under))
  {
    names.removeFirst();
  }
  int i = names.indexOf(QLatin1String("zlim")); // Currently the last function alphabetically

  while (i > 0 && i < names.size() && names.at(i).startsWith(QLatin1Char('z')))
  {
    // Check if there are more functions after zlim
    i++;
  }
  names.erase(names.begin() + i, names.end());
  qDebug() << "Received" << names.size() << "functions";
  addFunctions(names);

  // The list of functions from completion_matches('') includes keywords and variables too, so we have to re-add them
  addVariables(m_variables);
  addKeywords(m_keywords);
  rehighlight();
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

