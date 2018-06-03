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

#include <QGraphicsObject>
#include <QGraphicsSceneContextMenuEvent>

#include "worksheet.h"
#include "worksheettextitem.h"
#include "worksheetcursor.h"

class TextEntry;
class CommandEntry;
class ImageEntry;
class PageBreakEntry;
class LaTeXEntry;

class WorksheetTextItem;
class ActionBar;

class QPainter;
class QWidget;
class QPropertyAnimation;

struct AnimationData;

class WorksheetEntry : public QGraphicsObject
{
  Q_OBJECT
  public:
    WorksheetEntry(Worksheet* worksheet);
    ~WorksheetEntry() override;

    enum {Type = UserType};

    int type() const Q_DECL_OVERRIDE;

    virtual bool isEmpty()=0;

    static WorksheetEntry* create(int t, Worksheet* worksheet);

    WorksheetEntry* next() const;
    WorksheetEntry* previous() const;

    void setNext(WorksheetEntry*);
    void setPrevious(WorksheetEntry*);

    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) Q_DECL_OVERRIDE;

    virtual bool acceptRichText() = 0;

    virtual void setContent(const QString& content)=0;
    virtual void setContent(const QDomElement& content, const KZip& file)=0;

    virtual QDomElement toXml(QDomDocument& doc, KZip* archive)=0;
    virtual QString toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq)=0;

    virtual void interruptEvaluation()=0;

    virtual void showCompletion();

    virtual bool focusEntry(int pos = WorksheetTextItem::TopLeft, qreal xCoord = 0);

    virtual qreal setGeometry(qreal x, qreal y, qreal w);
    virtual void layOutForWidth(qreal w, bool force = false) = 0;
    QPropertyAnimation* sizeChangeAnimation(QSizeF s = QSizeF());

    virtual void populateMenu(QMenu *menu, const QPointF& pos);

    bool aboutToBeRemoved();
    QSizeF size();

    enum EvaluationOption {
        DoNothing, FocusNext, EvaluateNext
    };

    virtual WorksheetTextItem* highlightItem();

    bool hasActionBar();

    enum SearchFlag {SearchCommand=1, SearchResult=2, SearchError=4,
                     SearchText=8, SearchLaTeX=16, SearchAll=31};

    virtual WorksheetCursor search(QString pattern, unsigned flags,
                                   QTextDocument::FindFlags qt_flags,
                                   const WorksheetCursor& pos = WorksheetCursor());

  public Q_SLOTS:
    virtual bool evaluate(WorksheetEntry::EvaluationOption evalOp = FocusNext) = 0;
    virtual bool evaluateCurrentItem();
    virtual void updateEntry() = 0;

    void insertCommandEntry();
    void insertTextEntry();
    void insertLatexEntry();
    void insertImageEntry();
    void insertPageBreakEntry();
    void insertCommandEntryBefore();
    void insertTextEntryBefore();
    void insertLatexEntryBefore();
    void insertImageEntryBefore();
    void insertPageBreakEntryBefore();

    virtual void sizeAnimated();
    virtual void startRemoving();
    bool stopRemoving();
    void moveToPreviousEntry(int pos = WorksheetTextItem::BottomRight, qreal x = 0);
    void moveToNextEntry(int pos = WorksheetTextItem::TopLeft, qreal x = 0);
    void recalculateSize();

    // similar to recalculateSize, but the size change is animated
    void animateSizeChange();
    // animate the size change and the opacity of item
    void fadeInItem(QGraphicsObject* item = nullptr, const char* slot = nullptr);
    void fadeOutItem(QGraphicsObject* item = nullptr, const char* slot = "deleteLater()");
    void endAnimation();

    void showActionBar();
    void hideActionBar();

    void startDrag(const QPointF& grabPos = QPointF());

  Q_SIGNALS:
    void aboutToBeDeleted();

  protected:
    Worksheet* worksheet();
    WorksheetView* worksheetView();
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void evaluateNext(EvaluationOption opt);

    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) Q_DECL_OVERRIDE;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) Q_DECL_OVERRIDE;

    void setSize(QSizeF size);

    bool animationActive();
    void updateSizeAnimation(const QSizeF& size);

    void invokeSlotOnObject(const char* slot, QObject* obj);

    virtual void addActionsToBar(ActionBar* actionBar);

    virtual bool wantToEvaluate() = 0;
    virtual bool wantFocus();

  protected Q_SLOTS:
    virtual void remove();
    void deleteActionBar();
    void deleteActionBarAnimation();

  protected:
    static const qreal VerticalMargin;

  private:
    QSizeF m_size;
    WorksheetEntry* m_prev;
    WorksheetEntry* m_next;
    Q_PROPERTY(QSizeF size READ size WRITE setSize)
    AnimationData* m_animation;
    ActionBar* m_actionBar;
    QPropertyAnimation* m_actionBarAnimation;
    bool m_aboutToBeRemoved;
};

#endif // WORKSHEETENTRY_H
