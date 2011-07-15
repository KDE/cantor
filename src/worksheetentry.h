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
    Copyright (C) 2010 Raffaele De Feo <alberthilbert@gmail.com>
 */

#ifndef _WORKSHEETENTRY_H
#define _WORKSHEETENTRY_H

#include <QObject>
#include <QTextTableCell>
#include <QPointer>
#include <QKeyEvent>
#include <kmenu.h>
#include "lib/expression.h"

namespace Cantor{
    class Expression;
    class Result;
    class CompletionObject;
    class SyntaxHelpObject;
}
class Worksheet;
class KCompletionBox;

class WorksheetEntry : public QObject
{
  Q_OBJECT
  public:
    WorksheetEntry(QTextCursor position, Worksheet* parent);
    ~WorksheetEntry();

    enum {Type = 0};

    virtual int type();

    virtual bool isEmpty()=0;

    virtual void setActive(bool active, bool moveCursor);

    int firstPosition();
    int lastPosition();
    QTextCursor firstCursorPosition();
    QTextCursor lastCursorPosition();
    bool contains(const QTextCursor& cursor);

    virtual QTextCursor closestValidCursor(const QTextCursor& cursor)=0;
    virtual QTextCursor firstValidCursorPosition()=0;
    virtual QTextCursor lastValidCursorPosition()=0;
    int firstValidPosition();
    int lastValidPosition();
    virtual bool isValidCursor(const QTextCursor& cursor)=0;

    // Handlers for the worksheet input events affecting worksheetentries
    virtual bool worksheetShortcutOverrideEvent(QKeyEvent* event, const QTextCursor& cursor);
    virtual bool worksheetKeyPressEvent(QKeyEvent* event, const QTextCursor& cursor);
    virtual bool worksheetMousePressEvent(QMouseEvent* event, const QTextCursor& cursor);
    virtual bool worksheetContextMenuEvent(QContextMenuEvent* event, const QTextCursor& cursor);
    virtual bool worksheetMouseDoubleClickEvent(QMouseEvent* event, const QTextCursor& cursor);

    virtual bool acceptRichText()=0;
    virtual bool acceptsDrop(const QTextCursor& cursor)=0;

    virtual void setContent(const QString& content)=0;
    virtual void setContent(const QDomElement& content, const KZip& file)=0;

    virtual QDomElement toXml(QDomDocument& doc, KZip* archive)=0;
    virtual QString toPlain(QString& commandSep, QString& commentStartingSeq, QString& commentEndingSeq)=0;

    virtual void interruptEvaluation()=0;

    virtual bool evaluate(bool current)=0;

    virtual void checkForSanity();

    virtual void showCompletion();

  signals:
    void leftmostValidPositionReached();
    void rightmostValidPositionReached();
    void topmostValidLineReached();
    void bottommostValidLineReached();

  public slots:
    virtual void update()=0;

  protected:
    void createSubMenuInsert(KMenu* menu);

  protected:
    QTextFrame* m_frame;
    Worksheet* m_worksheet;
};

#endif /* _WORKSHEETENTRY_H */
