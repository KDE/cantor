/*
    SPDX-FileCopyrightText: 2010 Miha Čančula <miha.cancula@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef OCTAVEHIGHLIGHTER_H
#define OCTAVEHIGHLIGHTER_H

#include "defaulthighlighter.h"
#include <session.h>

namespace Cantor
{
    class Expression;
}

class OctaveHighlighter : public Cantor::DefaultHighlighter
{
  Q_OBJECT

  public:
    OctaveHighlighter(QObject* parent, Cantor::Session* session);
    ~OctaveHighlighter() override = default;
};

#endif // OCTAVEHIGHLIGHTER_H
