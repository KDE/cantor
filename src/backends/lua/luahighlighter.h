/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014 Lucas Hermann Negri <lucashnegri@gmail.com>
*/

#ifndef LUAHIGHLIGHTER_H
#define LUAHIGHLIGHTER_H

#include "defaulthighlighter.h"

class LuaHighlighter : public Cantor::DefaultHighlighter
{
public:
    explicit LuaHighlighter(QObject* parent);
    ~LuaHighlighter() override = default;
};

#endif // LUAHIGHLIGHTER_H
