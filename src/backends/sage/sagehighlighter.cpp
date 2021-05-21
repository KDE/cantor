/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "sagehighlighter.h"
#include "sagekeywords.h"

#include <QRegularExpression>

SageHighlighter::SageHighlighter(QObject* parent) : Cantor::DefaultHighlighter(parent)
{
    addRule(QRegularExpression(QStringLiteral("[A-Za-z0-9_]+(?=\\()")), functionFormat());

    addKeywords(SageKeywords::instance()->keywords());
    addFunctions(SageKeywords::instance()->functions());
    addVariables(SageKeywords::instance()->variables());

    addRule(QRegularExpression(QStringLiteral("#[^\n]*")), commentFormat());
}
