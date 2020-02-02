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
#include "octavesession.h"

using namespace Cantor;

OctaveHighlighter::OctaveHighlighter(QObject* parent, Session* session): DefaultHighlighter(parent, session)
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

  addRule(QRegularExpression(QStringLiteral("\"[^\"]*\"")), stringFormat());
  addRule(QRegularExpression(QStringLiteral("'[^']*'")), stringFormat());

  addRule(QRegularExpression(QStringLiteral("#[^\n]*")), commentFormat());
  addRule(QRegularExpression(QStringLiteral("%[^\n]*")), commentFormat());

  rehighlight();
}
