
#include "worksheettextitem.h"

WorksheetTextItem::WorksheetTextItem(QGraphicsItem* parent) 
  : QGraphicsTextItem(parent)
{
}

WorksheetTextItem::~WorksheetTextItem()
{
}

bool WorksheetTextItem::setCursorPosition(const QPointF& pos)
{
    QPointF localPos = mapFromParent(pos);
    int p = document()->documentLayout()->hitTest(localPos, Qt::FuzzyHit);
    QTextCursor cursor = textCursor();
    cursor.setPosition(p);
    setTextCursor(cursor);
}

QPointF WorksheetTextItem::cursorPosition() const
{
    QTextCursor cursor = textCursor();
    QTextBlock block = cursor.block();
    int p = cursor.position();
    QTextLine line = block.layout()->lineForTextPosition(p);
    QPointF localPos(line.cursorToX(p), line.y());
    return mapToParent(localPos);
}

Cantor::Session* WorksheetTextItem::session()
{
    return qt_cast<Worksheet*>(scene())->session();
}

void WorksheetTextItem::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Left:
	if (textCursor().atStart()) {
	    emit leftmostValidPositionReached();
	    kDebug()<<"Reached leftmost valid position";
	    return;
	}
	break;
    case Qt::Key_Right:
	if (textCursor().atEnd()) {
	    emit rightmostValidPositionReached();
	    kDebug()<<"Reached rightmost valid position";
	    return;
	}
	break;
    case Qt::Key_Up:
	if (!textCursor().movePosition(QTextCursor::Up)) {
	    emit topmostValidPositionReached();
	    kDebug()<<"Reached topmost valid position";
	    return;
	}
	break;
    case Qt::Key_Down:
	if (!textCursor().movePosition(QTextCursor::Down)) {
	    emit bottommostValidPositionReached();
	    kDebug()<<"Reached bottommost valid position";
	    return;
	}
	break;
    default:
	break;
    }
    this->QGraphicsTextItem::keyEvent(event);
}

void WorksheetTextItem::focusInEvent(QFocusEvent *event)
{
    emit receivedFocus(document());
}

#include "worksheettextitem.moc"
