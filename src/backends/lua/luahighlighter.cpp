/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014 Lucas Hermann Negri <lucashnegri@gmail.com>
*/

#include "luahighlighter.h"
#include "luakeywords.h"
#include "luahelper.h"

#include <QRegularExpression>

LuaHighlighter::LuaHighlighter(QObject* parent): DefaultHighlighter(parent)
{
    addKeywords (LuaKeywords::instance()->keywords());
    addFunctions(LuaKeywords::instance()->functions());
    addVariables(LuaKeywords::instance()->variables());

    addRule(QRegularExpression(QStringLiteral("[A-Za-z0-9_]+(?=\\()"))    , functionFormat());
    addRule(QRegularExpression(QStringLiteral("\"[^\"]*\""))              , stringFormat());
    addRule(QRegularExpression(QStringLiteral("'[^\'].*'"))               , stringFormat());
    addRule(QRegularExpression(QStringLiteral("--[^\n]*"))                , commentFormat());
    // did not add support for the multiline comment or multiline string
}
