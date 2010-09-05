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

#ifndef _COMMANDENTRY_H
#define _COMMANDENTRY_H

#include "worksheetentry.h"
#include <QObject>

class Worksheet;
class WorksheetEntry;

/**
   An entry in the Worksheet. it contains:
     1 Row to take command from the user
     0+ Rows for addition questions/answers from the backend
     0/1 Row for contextual help like Tab Completion offers
     1 Row for the Result
 **/

class CommandEntry : public WorksheetEntry
{
  Q_OBJECT
  public:
    static const QString Prompt;

    CommandEntry(QTextCursor position, Worksheet* parent);
    ~CommandEntry();

    enum {Type = 2};
    int type();

    QString command();
    void setExpression(Cantor::Expression* expr);
    Cantor::Expression* expression();

    //returns the line of the command cell, the textCursor is currently in
    QString currentLine(const QTextCursor& cursor);

    bool isEmpty();

    void setContent(const QString& content);
    void setContent(const QDomElement& content, const KZip& file);

    QDomElement toXml(QDomDocument& doc, KZip* archive);
    QString toPlain(QString& commandSep, QString& commentStartingSeq, QString& commentEndingSeq);

    void showCompletion();
    void setCompletion(Cantor::CompletionObject* tc);
    void setSyntaxHelp(Cantor::SyntaxHelpObject* sh);

    QTextTable* table();
    QTextTableCell commandCell();
    QTextTableCell actualInformationCell();
    QTextTableCell resultCell();

    void addInformation();

    QTextCursor closestValidCursor(const QTextCursor& cursor);
    QTextCursor firstValidCursorPosition();
    QTextCursor lastValidCursorPosition();
    bool isValidCursor(const QTextCursor& cursor);

    bool worksheetShortcutOverrideEvent(QKeyEvent* event, const QTextCursor& cursor);
    bool worksheetKeyPressEvent(QKeyEvent* event, const QTextCursor& cursor);
    bool worksheetMousePressEvent(QMouseEvent* event, const QTextCursor& cursor);
    bool worksheetContextMenuEvent(QContextMenuEvent* event, const QTextCursor& cursor);

    bool acceptRichText();
    bool acceptsDrop(const QTextCursor& cursor);

    bool isInCurrentInformationCell(const QTextCursor& cursor);
    bool isInCommandCell(const QTextCursor& cursor);
    bool isInPromptCell(const QTextCursor& cursor);
    bool isInResultCell(const QTextCursor& cursor);
    bool isInErrorCell(const QTextCursor& cursor);

    //checks if this entry has still anything needed (aka the user didn't delete anything
    //like the prompt. Readd missing things
    void checkForSanity();

    void removeContextHelp();
    void removeResult();

    bool evaluate(bool current);
    bool evaluateCommand();
    void interruptEvaluation();

    bool isShowingCompletionPopup();

  public slots:
    void update();
    void updatePrompt();
    void expressionChangedStatus(Cantor::Expression::Status status);
    void showAdditionalInformationPrompt(const QString& question);
    void showCompletions();
    void completeCommandTo(const QString& completion);
    void applySelectedCompletion();
    void showSyntaxHelp();
  private slots:
    void invalidate();
    void resultDeleted();

  private:
    QTextTable* m_table;
    QTextTableCell m_commandCell;
    QTextTableCell m_contextHelpCell;
    QList<QTextTableCell> m_informationCells;
    QTextTableCell m_errorCell;
    QTextTableCell m_resultCell;
    Cantor::Expression* m_expression;

    Cantor::CompletionObject* m_completionObject;
    QPointer<KCompletionBox> m_completionBox;
    Cantor::SyntaxHelpObject* m_syntaxHelpObject;
};

#endif /* _COMMANDENTRY_H */
