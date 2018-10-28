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
#include "octavekeywords.h"
#include "result.h"

#include <QDebug>
#include <QStringList>

using namespace Cantor;

OctaveHighlighter::OctaveHighlighter(QObject* parent, Session* session): DefaultHighlighter(parent), m_session(session)
{
  addKeywords(OctaveKeywords::instance()->keywords());
  addFunctions(OctaveKeywords::instance()->functions());

  QStringList operators;
  operators
    << QLatin1String("+") << QLatin1String("-") << QLatin1String("*") << QLatin1String("/")
    << QLatin1String(".+") << QLatin1String(".-") << QLatin1String(".*") << QLatin1String("./")
    << QLatin1String("=") << QLatin1String("or") << QLatin1String("and") << QLatin1String("xor")
    << QLatin1String("not") << QLatin1String("||") << QLatin1String("&&") << QLatin1String("==");
  addRules(operators, operatorFormat());

  addRule(QRegExp(QLatin1String("\"[^\"]*\"")), stringFormat());
  addRule(QRegExp(QLatin1String("'[^']*'")), stringFormat());

  addRule(QRegExp(QLatin1String("#[^\n]*")), commentFormat());
  addRule(QRegExp(QLatin1String("%[^\n]*")), commentFormat());

  rehighlight();
}

void OctaveHighlighter::updateVariables()
{
  Expression* expr = m_session->evaluateExpression(QLatin1String("who"), Expression::FinishingBehavior::DoNotDelete, true);
  connect(expr, &Expression::statusChanged, [this, expr](Expression::Status status)
  {
    if (status != Expression::Done && status != Expression::Error)
      return;

    if (status == Expression::Done)
    {
      QString res = expr->result()->toHtml();
      res.replace(QLatin1String("<br/>"),QLatin1String(" "));
      res.remove(0, res.indexOf(QLatin1Char('\n')));
      res.remove(QLatin1Char('\n'));
      res = res.trimmed();

      QStringList newVariables;
      foreach ( const QString& var, res.split(QLatin1Char(' '), QString::SkipEmptyParts))
      {
          newVariables <<  var.trimmed();
      }
      qDebug() << "Received" << newVariables.size() << "variables";

      for (const QString& newVariable: newVariables)
        if (!m_variables.contains(newVariable))
          addRule(newVariable, variableFormat());

      for (const QString& variable: m_variables)
        if (!newVariables.contains(variable))
          removeRule(variable);

      m_variables = std::move(newVariables);
      rehighlight();
    }
    expr->deleteLater();
  });
}

void OctaveHighlighter::sessionStatusChanged(Cantor::Session::Status status)
{
    if (status == Cantor::Session::Status::Disable)
        for (const QString& variable: m_variables)
            removeRule(variable);
    rehighlight();
}
