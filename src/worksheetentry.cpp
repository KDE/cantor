
#include "worksheetentry.h"
#include "commandentry.h"
#include "textentry.h"
#include "latexentry.h"

#include <KIcon>
#include <KLocale>

WorksheetEntry::WorksheetEntry(Worksheet* worksheet) : QGraphicsObject()
{
    Q_UNUSED(worksheet)

    m_next = 0;
    m_prev = 0;

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

qreal WorksheetEntry::setGeometry(qreal x, qreal y, qreal w)
{
    setPos(x, y);
    layOutForWidth(w);
    return size().height();
}

void WorksheetEntry::recalculateSize()
{
    layOutForWidth(size().width(), true);
    worksheet()->updateLayout();
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

    menu->addAction(i18n("Remove Entry"), this, SLOT(removeEntry()), 0);
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

void WorksheetEntry::removeEntry()
{
    if (previous())
	previous()->setNext(next());
    else
	worksheet()->setFirstEntry(next());
    if (next())
	next()->setPrevious(previous());
    else
	worksheet()->setLastEntry(previous());

    if (!next()) {
	if (previous() && previous()->isEmpty()) {
	    previous()->focusEntry();
	} else {
	    WorksheetEntry* next = worksheet()->appendCommandEntry();
	    next->focusEntry();
	}
    }
    deleteLater();
}

void WorksheetEntry::setSize(QSizeF size)
{
    m_size = size;
}

QSizeF WorksheetEntry::size()
{
    return m_size;
}

#include "worksheetentry.moc"
