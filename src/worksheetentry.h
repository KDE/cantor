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

#ifndef _WORKSHEETENTRY_H
#define _WORKSHEETENTRY_H

#include <QObject>
#include <QTextCursor>

class QTextDocument;
namespace MathematiK{
    class Expression;
}
class Worksheet;

class WorksheetEntry : public QObject
{
  Q_OBJECT
  public:
    static const QString Prompt;

    WorksheetEntry(QTextCursor cursor, Worksheet* parent);
    ~WorksheetEntry();

    QString command();
    void setExpression(MathematiK::Expression* expr);
    MathematiK::Expression* expression();

    int startPosition();
    int resultStartPosition();
    int endPosition();
    QTextCursor cmdCursor();
    QTextCursor resultCursor();
    QTextCursor endCursor();

    bool isEmpty();

    //only used for saving/loading. normally
    //you should create an expression, and set the result there
    void setResult(const QString& html);

    void setContextHelp(MathematiK::Expression* expression);

    //checks wether this cell is complete(contains the prompt etc)
    void checkForSanity();
  public slots:
    void updateResult();
    
    void showContextHelp();
  private slots:
    void resultDeleted();
  private:
    QTextFrame* m_cmdFrame;
    QTextFrame* m_resultFrame;

    MathematiK::Expression* m_expression;
    Worksheet* m_worksheet;

    MathematiK::Expression* m_contextHelpExpression;
};

#endif /* _WORKSHEETENTRY_H */
