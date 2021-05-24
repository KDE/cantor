/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
    SPDX-FileCopyrightText: 2018-2021 Alexander Semke <alexander.semke@web.de>
*/

#ifndef WORKSHEETENTRY_H
#define WORKSHEETENTRY_H

#include <QGraphicsObject>

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

class QGraphicsSceneContextMenuEvent;
class QJsonObject;
class QPainter;
class QPropertyAnimation;
class QWidget;

struct AnimationData;

class WorksheetEntry : public QGraphicsObject
{
  Q_OBJECT
  public:
    explicit WorksheetEntry(Worksheet*);
    ~WorksheetEntry() override;

    enum {Type = UserType};

    int type() const override;

    virtual bool isEmpty()=0;

    static WorksheetEntry* create(int t, Worksheet*);

    WorksheetEntry* next() const;
    WorksheetEntry* previous() const;

    void forceRemove();

    void setNext(WorksheetEntry*);
    void setPrevious(WorksheetEntry*);

    QRectF boundingRect() const override;
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;

    virtual bool acceptRichText() = 0;

    virtual void setContent(const QString&)=0;
    virtual void setContent(const QDomElement&, const KZip&)=0;
    virtual void setContentFromJupyter(const QJsonObject&)=0;

    virtual QDomElement toXml(QDomDocument&, KZip*)=0;
    virtual QJsonValue toJupyterJson()=0;
    virtual QString toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq)=0;

    virtual void interruptEvaluation()=0;

    virtual void showCompletion();

    virtual bool focusEntry(int pos = WorksheetTextItem::TopLeft, qreal xCoord = 0);

    virtual qreal setGeometry(qreal x, qreal entry_zone_x, qreal y, qreal w);
    virtual void layOutForWidth(qreal entry_zone_x, qreal w, bool force = false) = 0;
    QPropertyAnimation* sizeChangeAnimation(QSizeF s = QSizeF());

    virtual void populateMenu(QMenu*, QPointF);

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

    // Colors for colors menus;
    static constexpr int colorsCount = 26;
    static QColor colors[colorsCount];
    static QString colorNames[colorsCount];

    static const qreal VerticalMargin;
    static const qreal ControlElementWidth;
    static const qreal ControlElementBorder;
    static const qreal RightMargin;
    static const qreal HorizontalSpacing;

  public Q_SLOTS:
    virtual bool evaluate(WorksheetEntry::EvaluationOption evalOp = FocusNext) = 0;
    virtual bool evaluateCurrentItem();
    virtual void updateEntry() = 0;
    virtual void updateAfterSettingsChanges();

    void insertCommandEntry();
    void insertTextEntry();
    void insertMarkdownEntry();
    void insertLatexEntry();
    void insertImageEntry();
    void insertPageBreakEntry();
    void insertHorizontalRuleEntry();
    void insertHierarchyEntry();

    void insertCommandEntryBefore();
    void insertTextEntryBefore();
    void insertMarkdownEntryBefore();
    void insertLatexEntryBefore();
    void insertImageEntryBefore();
    void insertPageBreakEntryBefore();
    void insertHorizontalRuleEntryBefore();
    void insertHierarchyEntryBefore();

    void convertToCommandEntry();
    void convertToTextEntry();
    void convertToMarkdownEntry();
    void convertToLatexEntry();
    void convertToImageEntry();
    void converToPageBreakEntry();
    void convertToHorizontalRuleEntry();
    void convertToHierarchyEntry();

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

    virtual void startDrag(QPointF grabPos = QPointF());

    void moveToNext(bool updateLayout = true);
    void moveToPrevious(bool updateLayout = true);

  Q_SIGNALS:
    void aboutToBeDeleted();

  protected:
    Worksheet* worksheet();
    WorksheetView* worksheetView();
    void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void evaluateNext(EvaluationOption opt);

    void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;

    void setSize(QSizeF);

    bool animationActive();
    void updateSizeAnimation(QSizeF);

    void invokeSlotOnObject(const char* slot, QObject* obj);

    virtual void addActionsToBar(ActionBar*);

    virtual bool wantToEvaluate() = 0;
    virtual bool wantFocus();

    QJsonObject jupyterMetadata() const;
    void setJupyterMetadata(QJsonObject);

    virtual void recalculateControlGeometry();

    WorksheetControlItem m_controlElement;

  protected Q_SLOTS:
    virtual void remove();
    void deleteActionBar();
    void deleteActionBarAnimation();

  private:
    QSizeF m_size;
    qreal m_entry_zone_x{0.};
    WorksheetEntry* m_prev{nullptr};
    WorksheetEntry* m_next{nullptr};
    Q_PROPERTY(QSizeF size READ size WRITE setSize)
    AnimationData* m_animation{nullptr};
    ActionBar* m_actionBar{nullptr};
    QPropertyAnimation* m_actionBarAnimation{nullptr};
    bool m_aboutToBeRemoved{false};
    QJsonObject* m_jupyterMetadata{nullptr};
    bool m_isCellSelected{false};
};

#endif // WORKSHEETENTRY_H
