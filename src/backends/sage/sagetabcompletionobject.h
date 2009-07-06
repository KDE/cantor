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

#ifndef _SAGETABCOMPLETIONOBJECT_H
#define _SAGETABCOMPLETIONOBJECT_H

#include "tabcompletionobject.h"

class SageSession;

namespace MathematiK{
    class Expression;
};

class SageTabCompletionObject : public MathematiK::TabCompletionObject
{
  Q_OBJECT
  public:
    SageTabCompletionObject( const QString& command, SageSession* session);
    ~SageTabCompletionObject();

  protected slots:
    void fetchCompletions();
    void fetchingDone();

  private:
    MathematiK::Expression* m_expression;
};

#endif /* _SAGETABCOMPLETIONOBJECT_H */
