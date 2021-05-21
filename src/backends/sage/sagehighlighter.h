/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _SAGEHIGHLIGHTER_H
#define _SAGEHIGHLIGHTER_H

#include "defaulthighlighter.h"


/*
  this is basically a syntax highlighter for the
  Python programming Language, as Sage is based on
  it
*/

class SageHighlighter : public Cantor::DefaultHighlighter
{
  public:
    explicit SageHighlighter( QObject* parent);
    ~SageHighlighter() override = default;
};

#endif /* _SAGEHIGHLIGHTER_H */
