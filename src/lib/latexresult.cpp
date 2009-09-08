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
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
 */

#include "latexresult.h"
using namespace MathematiK;

class MathematiK::LatexResultPrivate
{
  public:
    QString code;
};

LatexResult::LatexResult(const QString& code, const KUrl& url) : EpsResult( url ),
                                                                 d(new LatexResultPrivate)
{
    d->code=code;
}

LatexResult::~LatexResult()
{
    delete d;
}

int LatexResult::type()
{
    return LatexResult::Type;
}

QString LatexResult::code()
{
    return d->code;
}
