/*
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

    ---
    Copyright (C) 2014 Lucas Hermann Negri <lucashnegri@gmail.com>
 */

#include "luahighlighter.h"
#include "luahelper.h"

LuaHighlighter::LuaHighlighter(QObject* parent): DefaultHighlighter(parent)
{
    addFunctions( luahelper_functions() );
    addKeywords ( luahelper_keywords()  );
    addVariables( luahelper_variables() );

    addRule(QRegExp("[A-Za-z0-9_]+(?=\\()")    , functionFormat());
    addRule(QRegExp("\".*\"")                  , stringFormat());
    addRule(QRegExp("'.*'")                    , stringFormat());
    addRule(QRegExp("--[^\n]*")                , commentFormat());
    // did not add support for the multiline comment or multiline string
}

LuaHighlighter::~LuaHighlighter()
{
}
