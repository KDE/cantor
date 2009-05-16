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
#include <QTextTableCell>

class QTextDocument;
namespace MathematiK{
    class Expression;
}
class Worksheet;

/**
   An entry in the Worksheet. it contains:
     1 Row to take command from the user
     0+ Rows for addition questions/answers from the backend
     1 Row for the Result
 **/

class WorksheetEntry : public QObject
{
  Q_OBJECT
  public:
    static const QString Prompt;

    WorksheetEntry(int position, Worksheet* parent);
    ~WorksheetEntry();

    QString command();
    void setExpression(MathematiK::Expression* expr);
    MathematiK::Expression* expression();

    bool isEmpty();

    //only used for saving/loading. normally
    //you should create an expression, and set the result there
    void setResult(const QString& html);

    void setContextHelp(MathematiK::Expression* expression);

    QTextTableCell commandCell();
    QTextTableCell actualInformationCell();
    QTextTableCell resultCell();

    void addInformation();

  public slots:
    void updateResult();
    void showAdditionalInformationPrompt(const QString& question);
    void showContextHelp();
  private slots:
    void resultDeleted();
  private:
    QTextTableCell m_commandCell;
    QList<QTextTableCell> m_informationCells;
    QTextTableCell m_resultCell;
    MathematiK::Expression* m_expression;
    Worksheet* m_worksheet;

    MathematiK::Expression* m_contextHelpExpression;
};

#endif /* _WORKSHEETENTRY_H */
