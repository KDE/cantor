/*
    SPDX-FileCopyrightText: 2010 Miha Čančula <miha.cancula@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "octavehighlighter.h"
#include "octavekeywords.h"
#include <session.h>

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
