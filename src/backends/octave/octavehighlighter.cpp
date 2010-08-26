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

#include <KDebug>

OctaveHighlighter::OctaveHighlighter(QTextEdit* parent, Cantor::Session* session): DefaultHighlighter(parent), m_session(session)
{
  updateFunctions();
  updateVariables();

  m_operators << "+" << "-" << "*" << "/" << ".+" << ".-" << ".*" << "./" << "=";
  m_operators << "or" << "and" << "xor" << "not";
  m_operators << "||" << "&&" << "==";
  addRules(m_operators, operatorFormat());

  m_keywords << "function" << "endfunction";
  m_keywords << "for" << "endfor";
  m_keywords << "while" << "endwhile";
  m_keywords << "if" << "endif" << "else" << "elseif";
  m_keywords << "switch" << "case" << "otherwise" << "endswitch";
  m_keywords << "end";
  addKeywords(m_keywords);

  addRule(QRegExp("\".*\""), stringFormat());
  addRule(QRegExp("'.*'"), stringFormat());

  rehighlight();
}

OctaveHighlighter::~OctaveHighlighter()
{

}

void OctaveHighlighter::updateFunctions()
{
  m_functionsExpr = m_session->evaluateExpression("completion_matches('')");
  connect ( m_functionsExpr, SIGNAL(statusChanged(Cantor::Expression::Status)), SLOT(receiveFunctions()) );
}

void OctaveHighlighter::updateVariables()
{
  m_varsExpr = m_session->evaluateExpression("who");
  connect ( m_varsExpr, SIGNAL(statusChanged(Cantor::Expression::Status)), SLOT(receiveVariables()) );
}


void OctaveHighlighter::receiveFunctions()
{
  kDebug();
  if (m_functionsExpr->status() != Cantor::Expression::Done || !m_functionsExpr->result())
  {
    return;
  }

  QStringList names = m_functionsExpr->result()->toHtml().split("<br/>\n");

  QLatin1String under("__");
  while (!names.first().contains(under))
  {
    names.removeFirst();
  }
  while (names.first().contains(under))
  {
    names.removeFirst();
  }
  int i = names.indexOf("zlim"); // Currently the last function alphabetically

  while (i > 0 && i < names.size() && names.at(i).startsWith('z'))
  {
    // Check if there are more functions after zlim
    i++;
  }
  names.erase(names.begin() + i, names.end());
  kDebug() << "Received" << names.size() << "functions";
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
  res.replace("<br/>"," ");
  res.remove(0, res.indexOf('\n'));
  res.remove('\n');
  res = res.trimmed();

  m_variables.clear();
  foreach ( const QString& var, res.split(' ', QString::SkipEmptyParts))
  {
      m_variables <<  var.trimmed();
  }
  kDebug() << "Received" << m_variables.size() << "variables";

  addVariables(m_variables);
  rehighlight();
}

