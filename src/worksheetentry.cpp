
#include "worksheetentry.h"
#include "commandentry.h"
#include "textentry.h"
#include "latexentry.h"

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QMetaMethod>

#include <KIcon>
#include <KLocale>
#include <kdebug.h>

struct AnimationData
{
    QAnimationGroup* animation;
    const char* slot;
    QGraphicsObject* item;
};

WorksheetEntry::WorksheetEntry(Worksheet* worksheet) : QGraphicsObject()
{
    Q_UNUSED(worksheet)

    m_next = 0;
    m_prev = 0;
    m_animation = 0;
    m_aboutToBeRemoved = false;
}

WorksheetEntry::~WorksheetEntry()
{
    if (next())
	next()->setPrevious(previous());
    if (previous())
	previous()->setNext(next());
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
    case CommandEntry::Type:
	return new CommandEntry(worksheet);
	/*
    case ImageEntry::Type:
	return new ImageEntry;
    case PageBreakEntry::Type:
	return new PageBreakEntry;
	*/
    case LatexEntry::Type:
	return new LatexEntry(worksheet);
    default:
	return 0;
    }
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
    Q_UNUSED(pos)
    Q_UNUSED(xCoord)
    return false;
}

void WorksheetEntry::moveToPreviousEntry(int pos, qreal x)
{
    if (previous())
	previous()->focusEntry(pos, x);
}

void WorksheetEntry::moveToNextEntry(int pos, qreal x)
{
    if (next())
	next()->focusEntry(pos, x);
}

Worksheet* WorksheetEntry::worksheet()
{
    return qobject_cast<Worksheet*>(scene());
}

void WorksheetEntry::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    KMenu *menu = worksheet()->createContextMenu();
    populateMenu(menu, event->pos());

    menu->popup(event->screenPos());
}

void WorksheetEntry::populateMenu(KMenu *menu, const QPointF& pos)
{
    if (!worksheet()->isRunning() && wantToEvaluate())
	menu->addAction(i18n("Evaluate Entry"), this, SLOT(evaluate()), 0);

    menu->addAction(i18n("Remove Entry"), this, SLOT(startRemoving()), 0);
    worksheet()->populateMenu(menu, mapToScene(pos));
}

void WorksheetEntry::evaluateNext(int opt)
{
    if (next()) {
	if (opt & EvaluateNextEntries) {
	    next()->evaluate(EvaluateNextEntries);
	} else {
	    worksheet()->setModified();
	    next()->focusEntry(WorksheetTextItem::BottomRight);
	}
    } else {
	if (!isEmpty() || type() != CommandEntry::Type)
	    worksheet()->appendCommandEntry();
	else
	    focusEntry();
	worksheet()->setModified();
    }
}

qreal WorksheetEntry::setGeometry(qreal x, qreal y, qreal w)
{
    setPos(x, y);
    layOutForWidth(w);
    return size().height();
}

void WorksheetEntry::recalculateSize()
{
    qreal height = size().height();
    layOutForWidth(size().width(), true);
    if (height != size().height())
	worksheet()->updateEntrySize(this);
}

QPropertyAnimation* WorksheetEntry::sizeChangeAnimation()
{
    QSizeF oldSize = size();
    layOutForWidth(size().width(), true);
    QSizeF newSize = size();
    kDebug() << oldSize << newSize;

    QPropertyAnimation* sizeAn = new QPropertyAnimation(this, "m_size", this);
    sizeAn->setDuration(200);
    sizeAn->setStartValue(oldSize);
    sizeAn->setEndValue(newSize);
    sizeAn->setEasingCurve(QEasingCurve::InOutQuad);
    connect(sizeAn, SIGNAL(valueChanged(const QVariant&)),
	    this, SLOT(sizeAnimated()));
    return sizeAn;
}

void WorksheetEntry::sizeAnimated()
{
    worksheet()->updateEntrySize(this);
}

void WorksheetEntry::animateSizeChange()
{
    if (m_animation) {
	layOutForWidth(size().width(), true);
	return;
    }
    QPropertyAnimation* sizeAn = sizeChangeAnimation();
    sizeAn->setEasingCurve(QEasingCurve::OutCubic);
    m_animation = new AnimationData;
    m_animation->item = 0;
    m_animation->slot = 0;
    m_animation->animation = new QParallelAnimationGroup(this);
    m_animation->animation->addAnimation(sizeAn);
    connect(m_animation->animation, SIGNAL(finished()),
	    this, SLOT(endAnimation()));
    m_animation->animation->start();
}

void WorksheetEntry::fadeInItem(QGraphicsObject* item, const char* slot)
{
    if (m_animation) {
	layOutForWidth(size().width(), true);
	return;
    }
    QPropertyAnimation* sizeAn = sizeChangeAnimation();
    sizeAn->setEasingCurve(QEasingCurve::OutCubic);
    if (slot)
	connect(sizeAn, SIGNAL(finished()), item, slot);
    QPropertyAnimation* opacAn = new QPropertyAnimation(item, "opacity", this);
    opacAn->setDuration(200);
    opacAn->setStartValue(0);
    opacAn->setEndValue(1);
    opacAn->setEasingCurve(QEasingCurve::OutCubic);

    m_animation = new AnimationData;
    m_animation->animation = new QParallelAnimationGroup(this);
    m_animation->item = item;
    m_animation->slot = slot;

    m_animation->animation->addAnimation(sizeAn);
    m_animation->animation->addAnimation(opacAn);
    connect(m_animation->animation, SIGNAL(finished()),
	    this, SLOT(endAnimation()));

    m_animation->animation->start();
}

void WorksheetEntry::fadeOutItem(QGraphicsObject* item, const char* slot)
{
    if (m_animation) {
	layOutForWidth(size().width(), true);
	return;
    }
    QPropertyAnimation* sizeAn = sizeChangeAnimation();
    if (slot)
	connect(sizeAn, SIGNAL(finished()), item, slot);
    QPropertyAnimation* opacAn = new QPropertyAnimation(item, "opacity", this);
    opacAn->setDuration(200);
    opacAn->setStartValue(1);
    opacAn->setEndValue(0);
    opacAn->setEasingCurve(QEasingCurve::OutCubic);

    m_animation = new AnimationData;
    m_animation->animation = new QParallelAnimationGroup(this);
    m_animation->item = item;
    m_animation->slot = slot;

    m_animation->animation->addAnimation(sizeAn);
    m_animation->animation->addAnimation(opacAn);
    connect(m_animation->animation, SIGNAL(finished()),
	    this, SLOT(endAnimation()));

    m_animation->animation->start();
}

void WorksheetEntry::endAnimation()
{
    if (!m_animation)
	return;
    QAnimationGroup* anim = m_animation->animation;
    if (anim->state() == QAbstractAnimation::Running) {
	anim->stop();
	QPropertyAnimation* sizeAn = qobject_cast<QPropertyAnimation*>(anim->animationAt(0));
	setSize(sizeAn->endValue().value<QSizeF>());

	if (anim->animationCount() == 2) {
	    QPropertyAnimation* opacAn = qobject_cast<QPropertyAnimation*>(anim->animationAt(1));
	    m_animation->item->setOpacity(opacAn->endValue().value<qreal>());
	}

	// If the animation was connected to a slot, call it
	if (m_animation->slot) {
	    const QMetaObject* metaObj = m_animation->item->metaObject();
	    int slotIndex = metaObj->indexOfSlot(m_animation->slot);
	    QMetaMethod method = metaObj->method(slotIndex);
	    method.invoke(m_animation->item, Qt::DirectConnection);
	}
    }
    m_animation->animation->deleteLater();
    delete m_animation;
    m_animation = 0;
}

bool WorksheetEntry::animationActive()
{
    return m_animation;
}

void WorksheetEntry::updateAnimation(const QSizeF& size)
{
    // Update the current animation, so that the new ending will be size

    if (m_aboutToBeRemoved)
	// do not modify the remove-animation
	return;
    QPropertyAnimation* sizeAn = qobject_cast<QPropertyAnimation*>(m_animation->animation->animationAt(0));
    qreal progress = static_cast<qreal>(sizeAn->currentTime()) / sizeAn->totalDuration();
    QEasingCurve curve = sizeAn->easingCurve();
    qreal value = curve.valueForProgress(progress);
    sizeAn->setEndValue(size);
    QSizeF newStart = 1/(1-value) * (sizeAn->currentValue().value<QSizeF>() -
				     value * size);
    sizeAn->setStartValue(newStart);
}

void WorksheetEntry::startRemoving()
{
    if (!next()) {
	if (previous() && previous()->isEmpty()) {
	    previous()->focusEntry();
	} else {
	    WorksheetEntry* next = worksheet()->appendCommandEntry();
	    setNext(next);
	    next->focusEntry();
	}
    } else {
	next()->focusEntry();
    }

    if (m_animation) {
	endAnimation();
    }

    m_aboutToBeRemoved = true;
    QPropertyAnimation* sizeAn = new QPropertyAnimation(this, "m_size", this);
    sizeAn->setDuration(300);
    sizeAn->setEndValue(QSizeF(size().width(), 0));
    sizeAn->setEasingCurve(QEasingCurve::InOutQuad);

    connect(sizeAn, SIGNAL(valueChanged(const QVariant&)),
	    this, SLOT(sizeAnimated()));
    connect(sizeAn, SIGNAL(finished()), this, SLOT(remove()));

    QPropertyAnimation* opacAn = new QPropertyAnimation(this, "opacity", this);
    opacAn->setDuration(300);
    opacAn->setEndValue(0);
    opacAn->setEasingCurve(QEasingCurve::OutCubic);

    m_animation = new AnimationData;
    m_animation->animation = new QParallelAnimationGroup(this);
    m_animation->animation->addAnimation(sizeAn);
    m_animation->animation->addAnimation(opacAn);

    m_animation->animation->start();
}

void WorksheetEntry::remove()
{
    if (previous())
	previous()->setNext(next());
    else
	worksheet()->setFirstEntry(next());
    if (next())
	next()->setPrevious(previous());
    else
	worksheet()->setLastEntry(previous());

    worksheet()->updateLayout();
    deleteLater();
}

void WorksheetEntry::setSize(QSizeF size)
{
    prepareGeometryChange();
    m_size = size;
}

QSizeF WorksheetEntry::size()
{
    return m_size;
}

#include "worksheetentry.moc"
