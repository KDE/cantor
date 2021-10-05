/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
    SPDX-FileCopyrightText: 2016-2021 Alexander Semke <alexander.semke@web.de>
*/

#include "worksheetentry.h"
#include "commandentry.h"
#include "textentry.h"
#include "markdownentry.h"
#include "latexentry.h"
#include "imageentry.h"
#include "pagebreakentry.h"
#include "horizontalruleentry.h"
#include "hierarchyentry.h"
#include "settings.h"
#include "actionbar.h"
#include "worksheettoolbutton.h"
#include "worksheetview.h"

#include <QDrag>
#include <QIcon>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QMetaMethod>
#include <QMimeData>
#include <QGraphicsProxyWidget>
#include <QBitmap>
#include <QJsonArray>
#include <QJsonObject>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>

#include <KColorScheme>
#include <KLocalizedString>
#include <KMessageBox>
#include <QDebug>

struct AnimationData
{
    QAnimationGroup* animation;
    QPropertyAnimation* sizeAnimation;
    QPropertyAnimation* opacAnimation;
    QPropertyAnimation* posAnimation;
    const char* slot;
    QGraphicsObject* item;
};

const qreal WorksheetEntry::VerticalMargin = 4;
const qreal WorksheetEntry::ControlElementWidth = 12;
const qreal WorksheetEntry::ControlElementBorder = 4;
const qreal WorksheetEntry::RightMargin = ControlElementWidth + 2*ControlElementBorder;
const qreal WorksheetEntry::HorizontalSpacing = 4;

QColor WorksheetEntry::colors[] = {QColor(255,255,255), QColor(0,0,0),
                                                    QColor(192,0,0), QColor(255,0,0), QColor(255,192,192), //red
                                                    QColor(0,192,0), QColor(0,255,0), QColor(192,255,192), //green
                                                    QColor(0,0,192), QColor(0,0,255), QColor(192,192,255), //blue
                                                    QColor(192,192,0), QColor(255,255,0), QColor(255,255,192), //yellow
                                                    QColor(0,192,192), QColor(0,255,255), QColor(192,255,255), //cyan
                                                    QColor(192,0,192), QColor(255,0,255), QColor(255,192,255), //magenta
                                                    QColor(192,88,0), QColor(255,128,0), QColor(255,168,88), //orange
                                                    QColor(128,128,128), QColor(160,160,160), QColor(195,195,195) //grey
                                                    };

QString WorksheetEntry::colorNames[] = {i18n("White"), i18n("Black"),
                                         i18n("Dark Red"), i18n("Red"), i18n("Light Red"),
                                         i18n("Dark Green"), i18n("Green"), i18n("Light Green"),
                                         i18n("Dark Blue"), i18n("Blue"), i18n("Light Blue"),
                                         i18n("Dark Yellow"), i18n("Yellow"), i18n("Light Yellow"),
                                         i18n("Dark Cyan"), i18n("Cyan"), i18n("Light Cyan"),
                                         i18n("Dark Magenta"), i18n("Magenta"), i18n("Light Magenta"),
                                         i18n("Dark Orange"), i18n("Orange"), i18n("Light Orange"),
                                         i18n("Dark Grey"), i18n("Grey"), i18n("Light Grey")
                                         };

WorksheetEntry::WorksheetEntry(Worksheet* worksheet) : QGraphicsObject(), m_controlElement(worksheet, this)
{
    worksheet->addItem(this);
    setAcceptHoverEvents(true);
    connect(&m_controlElement, &WorksheetControlItem::drag, this, &WorksheetEntry::startDrag);
}

WorksheetEntry::~WorksheetEntry()
{
    emit aboutToBeDeleted();
    if (next())
        next()->setPrevious(previous());
    if (previous())
        previous()->setNext(next());
    if (m_animation) {
        m_animation->animation->deleteLater();
        delete m_animation;
    }
    if (m_jupyterMetadata)
        delete m_jupyterMetadata;
    if (type() == HierarchyEntry::Type)
        worksheet()->updateHierarchyLayout();
}

int WorksheetEntry::type() const
{
    return Type;
}

WorksheetEntry* WorksheetEntry::create(int t, Worksheet* worksheet)
{
    switch(t)
    {
    case TextEntry::Type:
        return new TextEntry(worksheet);
    case MarkdownEntry::Type:
        return new MarkdownEntry(worksheet);
    case CommandEntry::Type:
        return new CommandEntry(worksheet);
    case ImageEntry::Type:
        return new ImageEntry(worksheet);
    case PageBreakEntry::Type:
        return new PageBreakEntry(worksheet);
    case LatexEntry::Type:
        return new LatexEntry(worksheet);
    case HorizontalRuleEntry::Type:
        return new HorizontalRuleEntry(worksheet);
    case HierarchyEntry::Type:
        return new HierarchyEntry(worksheet);
    default:
        return nullptr;
    }
}

void WorksheetEntry::insertCommandEntry()
{
    worksheet()->insertCommandEntry(this);
}

void WorksheetEntry::insertTextEntry()
{
    worksheet()->insertTextEntry(this);
}

void WorksheetEntry::insertMarkdownEntry()
{
    worksheet()->insertMarkdownEntry(this);
}

void WorksheetEntry::insertLatexEntry()
{
    worksheet()->insertLatexEntry(this);
}

void WorksheetEntry::insertImageEntry()
{
    worksheet()->insertImageEntry(this);
}

void WorksheetEntry::insertPageBreakEntry()
{
    worksheet()->insertPageBreakEntry(this);
}

void WorksheetEntry::insertHorizontalRuleEntry()
{
    worksheet()->insertHorizontalRuleEntry(this);
}

void WorksheetEntry::insertHierarchyEntry()
{
    worksheet()->insertHierarchyEntry(this);
}

void WorksheetEntry::insertCommandEntryBefore()
{
    worksheet()->insertCommandEntryBefore(this);
}

void WorksheetEntry::insertTextEntryBefore()
{
    worksheet()->insertTextEntryBefore(this);
}

void WorksheetEntry::insertMarkdownEntryBefore()
{
    worksheet()->insertMarkdownEntryBefore(this);
}

void WorksheetEntry::insertLatexEntryBefore()
{
    worksheet()->insertLatexEntryBefore(this);
}

void WorksheetEntry::insertImageEntryBefore()
{
    worksheet()->insertImageEntryBefore(this);
}

void WorksheetEntry::insertPageBreakEntryBefore()
{
    worksheet()->insertPageBreakEntryBefore(this);
}

void WorksheetEntry::insertHorizontalRuleEntryBefore()
{
    worksheet()->insertHorizontalRuleEntryBefore(this);
}

void WorksheetEntry::insertHierarchyEntryBefore()
{
    worksheet()->insertHierarchyEntryBefore(this);
}

void WorksheetEntry::convertToCommandEntry()
{
    worksheet()->changeEntryType(this, CommandEntry::Type);
}

void WorksheetEntry::convertToTextEntry()
{
    worksheet()->changeEntryType(this, TextEntry::Type);
}

void WorksheetEntry::convertToMarkdownEntry()
{
    worksheet()->changeEntryType(this, MarkdownEntry::Type);
}

void WorksheetEntry::convertToLatexEntry()
{
    worksheet()->changeEntryType(this, LatexEntry::Type);
}

void WorksheetEntry::convertToImageEntry()
{
    worksheet()->changeEntryType(this, ImageEntry::Type);
}

void WorksheetEntry::converToPageBreakEntry()
{
    worksheet()->changeEntryType(this, PageBreakEntry::Type);
}

void WorksheetEntry::convertToHorizontalRuleEntry()
{
    worksheet()->changeEntryType(this, HorizontalRuleEntry::Type);
}

void WorksheetEntry::convertToHierarchyEntry()
{
    worksheet()->changeEntryType(this, HierarchyEntry::Type);
}

void WorksheetEntry::showCompletion()
{
}

WorksheetEntry* WorksheetEntry::next() const
{
    return m_next;
}

WorksheetEntry* WorksheetEntry::previous() const
{
    return m_prev;
}

void WorksheetEntry::setNext(WorksheetEntry* n)
{
    m_next = n;
}

void WorksheetEntry::setPrevious(WorksheetEntry* p)
{
    m_prev = p;
}

void WorksheetEntry::startDrag(QPointF grabPos)
{
    // We need reset entry cursor manually, because otherwise the entry cursor will be visible on dragable item
    worksheet()->resetEntryCursor();

    QDrag* drag = new QDrag(worksheetView());
    qDebug() << size();
    const qreal scale = worksheet()->renderer()->scale();
    QPixmap pixmap((size()*scale).toSize());
    pixmap.fill(QColor(255, 255, 255, 0));
    QPainter painter(&pixmap);
    const QRectF sceneRect = mapRectToScene(boundingRect());
    worksheet()->render(&painter, pixmap.rect(), sceneRect);
    painter.end();
    QBitmap mask = pixmap.createMaskFromColor(QColor(255, 255, 255),
                                              Qt::MaskInColor);
    pixmap.setMask(mask);

    drag->setPixmap(pixmap);
    if (grabPos.isNull()) {
        const QPointF scenePos = worksheetView()->sceneCursorPos();
        drag->setHotSpot((mapFromScene(scenePos) * scale).toPoint());
    } else {
        drag->setHotSpot((grabPos * scale).toPoint());
    }
    drag->setMimeData(new QMimeData());

    worksheet()->startDrag(this, drag);
}


QRectF WorksheetEntry::boundingRect() const
{
    return QRectF(QPointF(0,0), m_size);
}

void WorksheetEntry::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(widget);
}

bool WorksheetEntry::focusEntry(int pos, qreal xCoord)
{
    Q_UNUSED(pos);
    Q_UNUSED(xCoord);

    if (flags() & QGraphicsItem::ItemIsFocusable) {
        setFocus();
        return true;
    }
    return false;
}

void WorksheetEntry::moveToPreviousEntry(int pos, qreal x)
{
    WorksheetEntry* entry = previous();
    while (entry && !(entry->wantFocus() && entry->focusEntry(pos, x)))
        entry = entry->previous();
}

void WorksheetEntry::moveToNextEntry(int pos, qreal x)
{
    WorksheetEntry* entry = next();
    while (entry && !(entry->wantFocus() && entry->focusEntry(pos, x)))
        entry = entry->next();
}

Worksheet* WorksheetEntry::worksheet()
{
    return qobject_cast<Worksheet*>(scene());
}

WorksheetView* WorksheetEntry::worksheetView()
{
    return worksheet()->worksheetView();
}

WorksheetCursor WorksheetEntry::search(const QString& pattern, unsigned flags,
                                   QTextDocument::FindFlags qt_flags,
                                   const WorksheetCursor& pos)
{
    Q_UNUSED(pattern);
    Q_UNUSED(flags);
    Q_UNUSED(qt_flags);
    Q_UNUSED(pos);

    return WorksheetCursor();
}

void WorksheetEntry::keyPressEvent(QKeyEvent* event)
{
    // This event is used in Entries that set the ItemIsFocusable flag
    switch(event->key()) {
    case Qt::Key_Left:
    case Qt::Key_Up:
        if (event->modifiers() == Qt::NoModifier)
            moveToPreviousEntry(WorksheetTextItem::BottomRight, 0);
        else if (event->modifiers() == Qt::CTRL)
            moveToPrevious();
        break;
    case Qt::Key_Right:
    case Qt::Key_Down:
        if (event->modifiers() == Qt::NoModifier)
            moveToNextEntry(WorksheetTextItem::TopLeft, 0);
        else if (event->modifiers() == Qt::CTRL)
            moveToNext();
        break;
        /*case Qt::Key_Enter:
    case Qt::Key_Return:
        if (event->modifiers() == Qt::ShiftModifier)
            evaluate();
        else if (event->modifiers() == Qt::ControlModifier)
            worksheet()->insertCommandEntry();
        break;*/
    default:
        event->ignore();
    }
}

void WorksheetEntry::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu *menu = worksheet()->createContextMenu();
    populateMenu(menu, event->pos());

    menu->popup(event->screenPos());
}

void WorksheetEntry::populateMenu(QMenu* menu, QPointF pos)
{
    QAction* firstAction = nullptr;
    if (!menu->actions().isEmpty()) //action() can be empty, s.a. WorksheetTextItem::populateMenu() where this function is called
        firstAction = menu->actions().first();

    QAction* action;
    if (!worksheet()->isRunning() && wantToEvaluate())
    {
        action = new QAction(QIcon::fromTheme(QLatin1String("media-playback-start")), i18n("Evaluate"));
        connect(action, SIGNAL(triggered()), this, SLOT(evaluate()));
        menu->insertAction(firstAction, action);
        menu->insertSeparator(firstAction);
    }

    if (m_prev) {
        action = new QAction(QIcon::fromTheme(QLatin1String("go-up")), i18n("Move Up"));
    //     connect(action, &QAction::triggered, this, &WorksheetEntry::moveToPrevious); //TODO: doesn't work
        connect(action, SIGNAL(triggered()), this, SLOT(moveToPrevious()));
        action->setShortcut(Qt::CTRL + Qt::Key_Up);
        menu->insertAction(firstAction, action);
    }

    if (m_next) {
        action = new QAction(QIcon::fromTheme(QLatin1String("go-down")), i18n("Move Down"));
    //     connect(action, &QAction::triggered, this, &WorksheetEntry::moveToNext); //TODO: doesn't work
        connect(action, SIGNAL(triggered()), this, SLOT(moveToNext()));
        action->setShortcut(Qt::CTRL + Qt::Key_Down);
        menu->insertAction(firstAction, action);
        menu->insertSeparator(firstAction);
    }

    action = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Remove"));
    connect(action, &QAction::triggered, this, &WorksheetEntry::startRemoving);
    action->setShortcut(Qt::ShiftModifier + Qt::Key_Delete);
    menu->insertAction(firstAction, action);
    menu->insertSeparator(firstAction);

    worksheet()->populateMenu(menu, mapToScene(pos));
}

bool WorksheetEntry::evaluateCurrentItem()
{
    // A default implementation that works well for most entries,
    // because they have only one item.
    return evaluate();
}

void WorksheetEntry::evaluateNext(EvaluationOption opt)
{
    // For cases, when code want *just* evaluate
    // the entry, for example, on load stage.
    // This internal evaluation shouldn't marked as
    // modifying change.
    if (opt == InternalEvaluation)
        return;

    WorksheetEntry* entry = next();

    while (entry && !entry->wantFocus())
        entry = entry->next();

    if (entry) {
        if (opt == EvaluateNext || Settings::self()->autoEval()) {
            entry->evaluate(EvaluateNext);
        } else if (opt == FocusNext) {
            worksheet()->setModified();
            entry->focusEntry(WorksheetTextItem::BottomRight);
        } else {
            worksheet()->setModified();
        }
    } else if (opt != DoNothing) {
        if (!worksheet()->isLoadingFromFile() && (!isEmpty() || type() != CommandEntry::Type))
            worksheet()->appendCommandEntry();
        else
            focusEntry();
        worksheet()->setModified();
    }
}

qreal WorksheetEntry::setGeometry(qreal x, qreal x1, qreal y, qreal w)
{
    setPos(x, y);
    m_entry_zone_x = x1;
    layOutForWidth(x1, w);

    recalculateControlGeometry();

    return size().height();
}

void WorksheetEntry::recalculateSize()
{
    qreal height = size().height();
    layOutForWidth(m_entry_zone_x, size().width(), true);
    if (height != size().height())
    {
        recalculateControlGeometry();
        worksheet()->updateEntrySize(this);
    }
}

QPropertyAnimation* WorksheetEntry::sizeChangeAnimation(QSizeF s)
{
    QSizeF oldSize;
    QSizeF newSize;
    if (s.isValid()) {
        oldSize = size();
        newSize = s;
    } else {
        oldSize = size();
        layOutForWidth(m_entry_zone_x, size().width(), true);
        newSize = size();
    }

    QPropertyAnimation* sizeAn = new QPropertyAnimation(this, "size", this);
    sizeAn->setDuration(200);
    sizeAn->setStartValue(oldSize);
    sizeAn->setEndValue(newSize);
    sizeAn->setEasingCurve(QEasingCurve::InOutQuad);
    connect(sizeAn, &QPropertyAnimation::valueChanged, this, &WorksheetEntry::sizeAnimated);
    return sizeAn;
}

void WorksheetEntry::sizeAnimated()
{
    recalculateControlGeometry();
    worksheet()->updateEntrySize(this);
}

void WorksheetEntry::animateSizeChange()
{
    if (!worksheet()->animationsEnabled()) {
        recalculateSize();
        return;
    }
    if (m_animation) {
        layOutForWidth(m_entry_zone_x, size().width(), true);
        return;
    }
    QPropertyAnimation* sizeAn = sizeChangeAnimation();
    m_animation = new AnimationData;
    m_animation->item = nullptr;
    m_animation->slot = nullptr;
    m_animation->opacAnimation = nullptr;
    m_animation->posAnimation = nullptr;
    m_animation->sizeAnimation = sizeAn;
    m_animation->sizeAnimation->setEasingCurve(QEasingCurve::OutCubic);
    m_animation->animation = new QParallelAnimationGroup(this);
    m_animation->animation->addAnimation(m_animation->sizeAnimation);
    connect(m_animation->animation, &QAnimationGroup::finished, this, &WorksheetEntry::endAnimation);
    m_animation->animation->start();
}

void WorksheetEntry::fadeInItem(QGraphicsObject* item, const char* slot)
{
    if (!worksheet()->animationsEnabled()) {
        recalculateSize();
        if (slot)
            invokeSlotOnObject(slot, item);
        return;
    }
    if (m_animation) {
        // this calculates the new size and calls updateSizeAnimation
        layOutForWidth(m_entry_zone_x, size().width(), true);
        if (slot)
            invokeSlotOnObject(slot, item);
        return;
    }
    QPropertyAnimation* sizeAn = sizeChangeAnimation();
    m_animation = new AnimationData;
    m_animation->sizeAnimation = sizeAn;
    m_animation->sizeAnimation->setEasingCurve(QEasingCurve::OutCubic);
    m_animation->opacAnimation = new QPropertyAnimation(item, "opacity", this);
    m_animation->opacAnimation->setDuration(200);
    m_animation->opacAnimation->setStartValue(0);
    m_animation->opacAnimation->setEndValue(1);
    m_animation->opacAnimation->setEasingCurve(QEasingCurve::OutCubic);
    m_animation->posAnimation = nullptr;

    m_animation->animation = new QParallelAnimationGroup(this);
    m_animation->item = item;
    m_animation->slot = slot;

    m_animation->animation->addAnimation(m_animation->sizeAnimation);
    m_animation->animation->addAnimation(m_animation->opacAnimation);

    connect(m_animation->animation, &QAnimationGroup::finished, this, &WorksheetEntry::endAnimation);

    m_animation->animation->start();
}

void WorksheetEntry::fadeOutItem(QGraphicsObject* item, const char* slot)
{
    // Note: The default value for slot is SLOT(deleteLater()), so item
    // will be deleted after the animation.
    if (!worksheet()->animationsEnabled()) {
        recalculateSize();
        if (slot)
            invokeSlotOnObject(slot, item);
        return;
    }
    if (m_animation) {
        // this calculates the new size and calls updateSizeAnimation
        layOutForWidth(m_entry_zone_x, size().width(), true);
        if (slot)
            invokeSlotOnObject(slot, item);
        return;
    }
    QPropertyAnimation* sizeAn = sizeChangeAnimation();
    m_animation = new AnimationData;
    m_animation->sizeAnimation = sizeAn;
    m_animation->opacAnimation = new QPropertyAnimation(item, "opacity", this);
    m_animation->opacAnimation->setDuration(200);
    m_animation->opacAnimation->setStartValue(1);
    m_animation->opacAnimation->setEndValue(0);
    m_animation->opacAnimation->setEasingCurve(QEasingCurve::OutCubic);
    m_animation->posAnimation = nullptr;

    m_animation->animation = new QParallelAnimationGroup(this);
    m_animation->item = item;
    m_animation->slot = slot;

    m_animation->animation->addAnimation(m_animation->sizeAnimation);
    m_animation->animation->addAnimation(m_animation->opacAnimation);

    connect(m_animation->animation, &QAnimationGroup::finished, this, &WorksheetEntry::endAnimation);

    m_animation->animation->start();
}

void WorksheetEntry::endAnimation()
{
    if (!m_animation)
        return;
    QAnimationGroup* anim = m_animation->animation;
    if (anim->state() == QAbstractAnimation::Running) {
        anim->stop();
        if (m_animation->sizeAnimation)
            setSize(m_animation->sizeAnimation->endValue().toSizeF());
        if (m_animation->opacAnimation) {
            qreal opac = m_animation->opacAnimation->endValue().value<qreal>();
            m_animation->item->setOpacity(opac);
        }
        if (m_animation->posAnimation) {
            const QPointF& pos = m_animation->posAnimation->endValue().toPointF();
            m_animation->item->setPos(pos);
        }

        // If the animation was connected to a slot, call it
        if (m_animation->slot)
            invokeSlotOnObject(m_animation->slot, m_animation->item);
    }
    m_animation->animation->deleteLater();
    delete m_animation;
    m_animation = nullptr;
}

bool WorksheetEntry::animationActive()
{
    return m_animation;
}

void WorksheetEntry::updateSizeAnimation(QSizeF size)
{
    // Update the current animation, so that the new ending will be size

    if (!m_animation)
        return;

    if (m_aboutToBeRemoved)
        // do not modify the remove-animation
        return;
    if (m_animation->sizeAnimation) {
        QPropertyAnimation* sizeAn = m_animation->sizeAnimation;
        qreal progress = static_cast<qreal>(sizeAn->currentTime()) /
            sizeAn->totalDuration();
        QEasingCurve curve = sizeAn->easingCurve();
        qreal value = curve.valueForProgress(progress);
        sizeAn->setEndValue(size);
        QSizeF newStart = 1/(1-value)*(sizeAn->currentValue().toSizeF() - value*size);
        sizeAn->setStartValue(newStart);
    } else {
        m_animation->sizeAnimation = sizeChangeAnimation(size);
        int d = m_animation->animation->duration() -
            m_animation->animation->currentTime();
        m_animation->sizeAnimation->setDuration(d);
        m_animation->animation->addAnimation(m_animation->sizeAnimation);
    }
}

void WorksheetEntry::invokeSlotOnObject(const char* slot, QObject* obj)
{
    const QMetaObject* metaObj = obj->metaObject();
    const QByteArray normSlot = QMetaObject::normalizedSignature(slot);
    const int slotIndex = metaObj->indexOfSlot(normSlot.constData());
    if (slotIndex == -1)
        qDebug() << "Warning: Tried to invoke an invalid slot:" << slot;
    const QMetaMethod method = metaObj->method(slotIndex);
    method.invoke(obj, Qt::DirectConnection);
}

bool WorksheetEntry::aboutToBeRemoved()
{
    return m_aboutToBeRemoved;
}

void WorksheetEntry::startRemoving()
{
    int rc = KMessageBox::warningYesNo(nullptr, i18n("Do you really want to remove this entry?"), i18n("Remove Entry"));
    if (rc == KMessageBox::No)
        return;

    if (!worksheet()->animationsEnabled()) {
        m_aboutToBeRemoved = true;
        remove();
        return;
    }
    if (m_aboutToBeRemoved)
        return;

    if (focusItem()) {
        if (!next()) {
            if (previous() && previous()->isEmpty() &&
                !previous()->aboutToBeRemoved()) {
                previous()->focusEntry();
            } else {
                WorksheetEntry* next = worksheet()->appendCommandEntry();
                setNext(next);
                next->focusEntry();
            }
        } else {
            next()->focusEntry();
        }
    }

    if (m_animation) {
        endAnimation();
    }

    m_aboutToBeRemoved = true;
    m_animation = new AnimationData;
    m_animation->sizeAnimation = new QPropertyAnimation(this, "size", this);
    m_animation->sizeAnimation->setDuration(300);
    m_animation->sizeAnimation->setEndValue(QSizeF(size().width(), 0));
    m_animation->sizeAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    connect(m_animation->sizeAnimation, &QPropertyAnimation::valueChanged, this, &WorksheetEntry::sizeAnimated);
    connect(m_animation->sizeAnimation, &QPropertyAnimation::finished, this, &WorksheetEntry::remove);

    m_animation->opacAnimation = new QPropertyAnimation(this, "opacity", this);
    m_animation->opacAnimation->setDuration(300);
    m_animation->opacAnimation->setEndValue(0);
    m_animation->opacAnimation->setEasingCurve(QEasingCurve::OutCubic);
    m_animation->posAnimation = nullptr;

    m_animation->animation = new QParallelAnimationGroup(this);
    m_animation->animation->addAnimation(m_animation->sizeAnimation);
    m_animation->animation->addAnimation(m_animation->opacAnimation);

    m_animation->animation->start();
}

bool WorksheetEntry::stopRemoving()
{
    if (!m_aboutToBeRemoved)
        return true;

    if (m_animation->animation->state() == QAbstractAnimation::Stopped)
        // we are too late to stop the deletion
        return false;

    m_aboutToBeRemoved = false;
    m_animation->animation->stop();
    m_animation->animation->deleteLater();
    delete m_animation;
    m_animation = nullptr;
    return true;
}

void WorksheetEntry::remove()
{
    if (!m_aboutToBeRemoved)
        return;

    if (previous() && previous()->next() == this)
        previous()->setNext(next());
    else
        worksheet()->setFirstEntry(next());
    if (next() && next()->previous() == this)
        next()->setPrevious(previous());
    else
        worksheet()->setLastEntry(previous());

    if (type() == HierarchyEntry::Type)
        worksheet()->updateHierarchyLayout();

    // make the entry invisible to QGraphicsScene's itemAt() function
    forceRemove();

    worksheet()->setModified();
}

void WorksheetEntry::setSize(QSizeF size)
{
    prepareGeometryChange();
    if (m_actionBar && size != m_size)
        m_actionBar->updatePosition();
    m_size = size;
}

QSizeF WorksheetEntry::size()
{
    return m_size;
}

bool WorksheetEntry::hasActionBar()
{
    return m_actionBar;
}

void WorksheetEntry::showActionBar()
{
    if (m_actionBar && !m_actionBarAnimation)
        return;

    if (m_actionBarAnimation) {
        if (m_actionBarAnimation->endValue().toReal() == 1)
            return;
        m_actionBarAnimation->stop();
        delete m_actionBarAnimation;
        m_actionBarAnimation = nullptr;
    }

    if (!m_actionBar) {
        m_actionBar = new ActionBar(this);

        m_actionBar->addButton(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Remove Entry"),
                               this, SLOT(startRemoving()));

        WorksheetToolButton* dragButton;
        dragButton = m_actionBar->addButton(QIcon::fromTheme(QLatin1String("transform-move")),
                                            i18n("Drag Entry"));
        connect(dragButton, SIGNAL(pressed()), this, SLOT(startDrag()));

        if (wantToEvaluate()) {
            QString toolTip = i18n("Evaluate Entry");
            m_actionBar->addButton(QIcon::fromTheme(QLatin1String("media-playback-start")), toolTip,
                                   this, SLOT(evaluate()));
        }

        m_actionBar->addSpace();

        addActionsToBar(m_actionBar);
    }

    if (worksheet()->animationsEnabled()) {
        m_actionBarAnimation = new QPropertyAnimation(m_actionBar, "opacity",
                                                      this);
        m_actionBarAnimation->setStartValue(0);
        m_actionBarAnimation->setKeyValueAt(0.666, 0);
        m_actionBarAnimation->setEndValue(1);
        m_actionBarAnimation->setDuration(600);
        connect(m_actionBarAnimation, &QPropertyAnimation::finished, this, &WorksheetEntry::deleteActionBarAnimation);

        m_actionBarAnimation->start();
    }
}

void WorksheetEntry::hideActionBar()
{
    if (!m_actionBar)
        return;

    if (m_actionBarAnimation) {
        if (m_actionBarAnimation->endValue().toReal() == 0)
            return;
        m_actionBarAnimation->stop();
        delete m_actionBarAnimation;
        m_actionBarAnimation = nullptr;
    }

    if (worksheet()->animationsEnabled()) {
        m_actionBarAnimation = new QPropertyAnimation(m_actionBar, "opacity",
                                                      this);
        m_actionBarAnimation->setEndValue(0);
        m_actionBarAnimation->setEasingCurve(QEasingCurve::Linear);
        m_actionBarAnimation->setDuration(200);
        connect(m_actionBarAnimation, &QPropertyAnimation::finished, this, &WorksheetEntry::deleteActionBar);

        m_actionBarAnimation->start();
    } else {
        deleteActionBar();
    }
}

void WorksheetEntry::deleteActionBarAnimation()
{
    if (m_actionBarAnimation) {
        delete m_actionBarAnimation;
        m_actionBarAnimation = nullptr;
    }
}

void WorksheetEntry::deleteActionBar()
{
    if (m_actionBar) {
        delete m_actionBar;
        m_actionBar = nullptr;
    }

    deleteActionBarAnimation();
}

void WorksheetEntry::addActionsToBar(ActionBar*)
{
}

void WorksheetEntry::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event);
    showActionBar();
}

void WorksheetEntry::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event);
    hideActionBar();
}

WorksheetTextItem* WorksheetEntry::highlightItem()
{
    return nullptr;
}

bool WorksheetEntry::wantFocus()
{
    return true;
}

QJsonObject WorksheetEntry::jupyterMetadata() const
{
    return m_jupyterMetadata ? *m_jupyterMetadata : QJsonObject();
}

void WorksheetEntry::setJupyterMetadata(QJsonObject metadata)
{
    if (m_jupyterMetadata == nullptr)
        m_jupyterMetadata = new QJsonObject();
    *m_jupyterMetadata = metadata;
}

void WorksheetEntry::forceRemove()
{
    hide();
    worksheet()->updateLayout();
    deleteLater();
}

bool WorksheetEntry::isCellSelected()
{
    return m_controlElement.isSelected;
}

void WorksheetEntry::setCellSelected(bool val)
{
    m_controlElement.isSelected = val;
}

void WorksheetEntry::moveToNext(bool updateLayout)
{
    WorksheetEntry* next = this->next();
    if (next)
    {
        if (next->next())
        {
            next->next()->setPrevious(this);
            this->setNext(next->next());
        }
        else
        {
            worksheet()->setLastEntry(this);
            this->setNext(nullptr);
        }

        next->setPrevious(this->previous());
        next->setNext(this);

        this->setPrevious(next);
        if (next->previous())
            next->previous()->setNext(next);
        else
            worksheet()->setFirstEntry(next);

        if (updateLayout)
            worksheet()->updateLayout();

        worksheet()->setModified();
    }
}

void WorksheetEntry::moveToPrevious(bool updateLayout)
{
    WorksheetEntry* previous = this->previous();
    if (previous)
    {
        if (previous->previous())
        {
            previous->previous()->setNext(this);
            this->setPrevious(previous->previous());
        }
        else
        {
            worksheet()->setFirstEntry(this);
            this->setPrevious(nullptr);
        }

        previous->setNext(this->next());
        previous->setPrevious(this);

        this->setNext(previous);
        if (previous->next())
            previous->next()->setPrevious(previous);
        else
            worksheet()->setLastEntry(previous);

        if (updateLayout)
            worksheet()->updateLayout();

        worksheet()->setModified();
    }
}

void WorksheetEntry::recalculateControlGeometry()
{
    m_controlElement.setRect(
        size().width() - ControlElementWidth - ControlElementBorder, 0, // x,y
        ControlElementWidth, size().height() - VerticalMargin // w,h
    );
    m_controlElement.update();
}

void WorksheetEntry::updateAfterSettingsChanges()
{
    // do nothing;
}
