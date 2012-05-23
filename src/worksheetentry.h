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
    Copyright (C) 2012 Martin Kuettler <martin.kuettler@gmail.com>
 */

#ifndef WORKSHEETENTRY_H
#define WORKSHEETENTRY_H

#include <QGraphicsWidget>
#include <QGraphicsSceneContextMenuEvent>

#include "worksheet.h"
#include "worksheettextitem.h"

class TextEntry;
class CommandEntry;
class ImageEntry;
class PageBreakEntry;
class LaTeXEntry;

class WorksheetEntry : public QGraphicsWidget
{
  Q_OBJECT
  public:
    WorksheetEntry(Worksheet* worksheet);
    virtual ~WorksheetEntry();

    enum {Type = UserType};

    virtual int type() const;

    virtual bool isEmpty()=0;

    static WorksheetEntry* create(int t, Worksheet* worksheet);

    WorksheetEntry* next() const;
    WorksheetEntry* previous() const;

    void setNext(WorksheetEntry*);
    void setPrevious(WorksheetEntry*);

    QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint) const;
    virtual bool acceptRichText() = 0;

    virtual void setContent(const QString& content)=0;
    virtual void setContent(const QDomElement& content, const KZip& file)=0;

    virtual QDomElement toXml(QDomDocument& doc, KZip* archive)=0;
    virtual QString toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq)=0;

    virtual void interruptEvaluation()=0;

    virtual void showCompletion();

    virtual bool focusEntry(int pos = WorksheetTextItem::TopLeft, qreal xCoord = 0);

    virtual void layOutForWidth(double w, bool force = false) = 0;

    virtual void populateMenu(KMenu *menu);

    enum EvaluationOption {
	FocusedItemOnly = 1,
	EvaluateNextEntries = 2
    };

  public slots:
    virtual bool evaluate(int evalOp = 0) = 0;
    virtual void updateEntry() = 0;
    void moveToPreviousEntry(int pos = WorksheetTextItem::BottomRight, qreal x = 0);
    void moveToNextEntry(int pos = WorksheetTextItem::TopLeft, qreal x = 0);
    void recalculateSize();

  protected:
    Worksheet* worksheet();
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void evaluateNext(int opt);

    void setEntrySize(QSizeF size);
    QSizeF entrySize();

    virtual bool wantToEvaluate() = 0;

  private:
    QSizeF m_size;
    WorksheetEntry* m_prev;
    WorksheetEntry* m_next;
};

#endif // WORKSHEETENTRY_H
