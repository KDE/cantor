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

#ifndef OCTAVEHIGHLIGHTER_H
#define OCTAVEHIGHLIGHTER_H

#include "defaulthighlighter.h"

namespace Cantor
{
    class Session;
    class Expression;
}

class OctaveHighlighter : public Cantor::DefaultHighlighter
{
  Q_OBJECT
  
  public:
    OctaveHighlighter(QTextEdit* parent, Cantor::Session* session);
    virtual ~OctaveHighlighter();

  
protected:
    virtual void highlightBlock(const QString& text);
    
  public Q_SLOTS:
    void receiveFunctions();
    void receiveVariables();
    
    void updateFunctions();
    void updateVariables();
    
  private:
    QStringList m_functions;
    QStringList m_variables;
    QStringList m_operators;
    QStringList m_keywords;
    
    Cantor::Session* m_session;
    Cantor::Expression* m_functionsExpr;
    Cantor::Expression* m_varsExpr;
};

#endif // OCTAVEHIGHLIGHTER_H
