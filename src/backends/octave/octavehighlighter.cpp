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

OctaveHighlighter::OctaveHighlighter(QTextEdit* parent, Cantor::Session* session): DefaultHighlighter(parent)
, m_session(session)
{
  updateFunctions();
  updateVariables();
  
  m_operators << "\\+" << "\\-" << "\\*" << "\\/" << "\\.\\+" << "\\.\\-" << "\\.\\*" << "\\.\\/" << "\\=";
  m_operators << "\\bor\\b" << "\\band\\b" << "\\bxor\\b" << "\\bnot\\b";
  m_operators << "\\|\\|" << "\\&\\&" << "\\=\\=";
    
  m_keywords << "\\bfunction\\b" << "\\bendfunction\\b";
  m_keywords << "\\bfor\\b" << "\\bendfor\\b";
  m_keywords << "\\bwhile\\b" << "\\bendwhile\\b";
  m_keywords << "\\bif\\b" << "\\bendif\\b" << "\\belse\\b" << "\\belseif\\b";
  m_keywords << "\\bswitch\\b" << "\\bcase\\b" << "\\botherwise\\b" << "\\bendswitch\\b";
  m_keywords << "\\bend\\b";
}

OctaveHighlighter::~OctaveHighlighter()
{

}

void OctaveHighlighter::highlightBlock(const QString& text)
{
    if ( text.isEmpty() )
    {
      return;
    }
    
    DefaultHighlighter::highlightBlock(text);

    foreach (const QString& f, m_functions)
    {
	// WARNING: There are 1600 functions, this might be slow
	QRegExp expression(f);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index,  length,  functionFormat());
            index = expression.indexIn(text,  index + length);
        }
    }
    foreach (const QString& v, m_variables)
    {
	QRegExp expression(v);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index,  length,  variableFormat());
            index = expression.indexIn(text,  index + length);
        }
    }
    foreach (const QString& op, m_operators)
    {
	QRegExp expression(op);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index,  length,  operatorFormat());
            index = expression.indexIn(text,  index + length);
        }
    }
    foreach (const QString& k, m_keywords)
    {
	QRegExp expression(k);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index,  length,  keywordFormat());
            index = expression.indexIn(text,  index + length);
        }
    }
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
  while (!names.first().contains(under))
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
  foreach (const QString& name, names)
  {
    m_functions << "\\b" + name + "\\b";
  }
  kDebug() << m_functions.size();
  rehighlight();
}

void OctaveHighlighter::receiveVariables()
{
  if (m_varsExpr->status() != Cantor::Expression::Done || !m_varsExpr->result())
  {
    return;
  }
  QString res = m_varsExpr->result()->toHtml();
  res.remove("<br/>");
  res.remove(0, res.indexOf('\n'));
  res = res.trimmed();
  foreach ( const QString& var, res.split(' ', QString::SkipEmptyParts))
  {
    m_variables << "\\b" + var.trimmed() + "\\b";
  }
  kDebug() << m_variables;
  rehighlight();
}

