
#include "worksheettextitem.h"

WorksheetTextItem::WorksheetTextItem() : QGraphicsTextItem()
{
    m_highlighter = 0;
}

WorksheetTextItem::~WorksheetTextItem()
{
}

void WorksheetTextItem::enableHighlighting(bool highlight)
{
    if (!highlight) {
	if (m_highlighter)
	    m_highlighter->deleteLater();
	m_highlighter = 0;
    } else {
	if (!m_highlighter)
	    m_highlighter = session()->syntaxHighlighter();
	if (!m_highlighter)
	    m_highlighter = new Cantor::DefaultHighlighter;
	m_highlighter.setDocument(document());
    }
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
	    return true;
	}
	break;
    case Qt::Key_Right:
	if (textCursor().atEnd()) {
	    emit rightmostValidPositionReached();
	    kDebug()<<"Reached rightmost valid position";
	    return true;
	}
	break;
    case Qt::Key_Up:
	if (!textCursor().movePosition(QTextCursor::Up)) {
	    emit topmostValidPositionReached();
	    kDebug()<<"Reached topmost valid position";
	    return true;
	}
	break;
    case Qt::Key_Down:
	if (!textCursor().movePosition(QTextCursor::Down)) {
	    emit bottommostValidPositionReached();
	    kDebug()<<"Reached bottommost valid position";
	    return true;
	}
	break;
    default:
	break;
    }
    return this->QGraphicsTextItem::keyEvent(event);
}

#include "worksheettextitem.moc"
