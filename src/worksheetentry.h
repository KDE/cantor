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
#include <QGraphicsRectItem>
#include <QGraphicsSceneContextMenuEvent>

#include "worksheet.h"
#include "worksheettextitem.h"
#include "worksheetcursor.h"
#include "worksheetcontrolitem.h"

class TextEntry;
class MarkdownEntry;
class CommandEntry;
class ImageEntry;
class PageBreakEntry;
class LaTeXEntry;

class WorksheetTextItem;
class ActionBar;

class QPainter;
class QWidget;
class QPropertyAnimation;
class QJsonObject;

struct AnimationData;

class WorksheetEntry : public QGraphicsObject
{
  Q_OBJECT
  public:
    explicit WorksheetEntry(Worksheet* worksheet);
    ~WorksheetEntry() override;

    enum {Type = UserType};

    int type() const override;

    virtual bool isEmpty()=0;

    static WorksheetEntry* create(int t, Worksheet* worksheet);

    WorksheetEntry* next() const;
    WorksheetEntry* previous() const;

    void forceRemove();

    void setNext(WorksheetEntry*);
    void setPrevious(WorksheetEntry*);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    virtual bool acceptRichText() = 0;

    virtual void setContent(const QString& content)=0;
    virtual void setContent(const QDomElement& content, const KZip& file)=0;
    virtual void setContentFromJupyter(const QJsonObject& cell)=0;

    virtual QDomElement toXml(QDomDocument& doc, KZip* archive)=0;
    virtual QJsonValue toJupyterJson()=0;
    virtual QString toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq)=0;

    virtual void interruptEvaluation()=0;

    virtual void showCompletion();

    virtual bool focusEntry(int pos = WorksheetTextItem::TopLeft, qreal xCoord = 0);

    virtual qreal setGeometry(qreal x, qreal entry_zone_x, qreal y, qreal w);
    virtual void layOutForWidth(qreal entry_zone_x, qreal w, bool force = false) = 0;
    QPropertyAnimation* sizeChangeAnimation(QSizeF s = QSizeF());

    virtual void populateMenu(QMenu* menu, QPointF pos);

    bool aboutToBeRemoved();
    QSizeF size();

    enum EvaluationOption {
        InternalEvaluation, DoNothing, FocusNext, EvaluateNext
    };

    virtual WorksheetTextItem* highlightItem();

    bool hasActionBar();

    enum SearchFlag {SearchCommand=1, SearchResult=2, SearchError=4,
                     SearchText=8, SearchLaTeX=16, SearchAll=31};

    virtual WorksheetCursor search(const QString& pattern, unsigned flags,
                                   QTextDocument::FindFlags qt_flags,
                                   const WorksheetCursor& pos = WorksheetCursor());

    bool isCellSelected();
    void setCellSelected(bool);

  public Q_SLOTS:
    virtual bool evaluate(WorksheetEntry::EvaluationOption evalOp = FocusNext) = 0;
    virtual bool evaluateCurrentItem();
    virtual void updateEntry() = 0;

    void insertCommandEntry();
    void insertTextEntry();
    void insertMarkdownEntry();
    void insertLatexEntry();
    void insertImageEntry();
    void insertPageBreakEntry();

    void insertCommandEntryBefore();
    void insertTextEntryBefore();
    void insertMarkdownEntryBefore();
    void insertLatexEntryBefore();
    void insertImageEntryBefore();
    void insertPageBreakEntryBefore();

    void convertToCommandEntry();
    void convertToTextEntry();
    void convertToMarkdownEntry();
    void convertToLatexEntry();
    void convertToImageEntry();
    void converToPageBreakEntry();

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

    void startDrag(QPointF grabPos = QPointF());

    void moveToNext(bool updateLayout = true);
    void moveToPrevious(bool updateLayout = true);

  Q_SIGNALS:
    void aboutToBeDeleted();

  protected:
    Worksheet* worksheet();
    WorksheetView* worksheetView();
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void evaluateNext(EvaluationOption opt);

    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

    void setSize(QSizeF size);

    bool animationActive();
    void updateSizeAnimation(QSizeF size);

    void invokeSlotOnObject(const char* slot, QObject* obj);

    virtual void addActionsToBar(ActionBar* actionBar);

    virtual bool wantToEvaluate() = 0;
    virtual bool wantFocus();

    QJsonObject jupyterMetadata() const;
    void setJupyterMetadata(QJsonObject metadata);

    void recalculateControlGeometry();

  protected Q_SLOTS:
    virtual void remove();
    void deleteActionBar();
    void deleteActionBarAnimation();

  public:
    static const qreal VerticalMargin;
    static const qreal ControlElementWidth;
    static const qreal ControlElementBorder;
    static const qreal RightMargin;

  protected:
    WorksheetControlItem m_controlElement;

  private:
    QSizeF m_size;
    WorksheetEntry* m_prev;
    WorksheetEntry* m_next;
    Q_PROPERTY(QSizeF size READ size WRITE setSize)
    AnimationData* m_animation;
    ActionBar* m_actionBar;
    QPropertyAnimation* m_actionBarAnimation;
    bool m_aboutToBeRemoved;
    QJsonObject* m_jupyterMetadata;
    bool m_isCellSelected{false};
};

#endif // WORKSHEETENTRY_H
