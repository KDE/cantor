
#include "worksheettextitem.h"
#include "worksheet.h"

#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextLine>

#include <kdebug.h>

WorksheetTextItem::WorksheetTextItem(QGraphicsWidget* parent, QGraphicsLayoutItem* lparent) 
    : WorksheetStaticTextItem(parent, lparent)
{
    setTextInteractionFlags(Qt::TextEditorInteraction);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
}

WorksheetTextItem::~WorksheetTextItem()
{
}

void WorksheetTextItem::setCursorPosition(const QPointF& pos)
{
    setLocalCursorPosition(mapFromParent(pos));
}

QPointF WorksheetTextItem::cursorPosition() const
{
    return mapToParent(localCursorPosition());
}

void WorksheetTextItem::setLocalCursorPosition(const QPointF& pos)
{
    int p = document()->documentLayout()->hitTest(pos, Qt::FuzzyHit);
    QTextCursor cursor = textCursor();
    cursor.setPosition(p);
    setTextCursor(cursor);
}

QPointF WorksheetTextItem::localCursorPosition() const
{
    QTextCursor cursor = textCursor();
    QTextBlock block = cursor.block();
    int p = cursor.position();
    QTextLine line = block.layout()->lineForTextPosition(p);
    return QPointF(line.cursorToX(p), line.y());
}

void WorksheetTextItem::setEditable(bool e)
{
    if (e)
	setTextInteractionFlags(Qt::TextEditorInteraction);
    else
	setTextInteractionFlags(Qt::TextSelectableByMouse);
}

bool WorksheetTextItem::isEditable()
{
    return textInteractionFlags() & Qt::TextEditable;
}

void WorksheetTextItem::focusItem(int pos, qreal xCoord)
{
    QTextCursor cursor = textCursor();
    if (pos == TopLeft) {
	cursor.movePosition(QTextCursor::Start);
    } else if (pos == BottomRight) {
	cursor.movePosition(QTextCursor::End);
    } else {
	QTextLine line;
	if (pos == TopCoord) {
	    line = document()->firstBlock().layout()->lineAt(0);
	} else {
	    QTextLayout* layout = document()->lastBlock().layout();
	    line = layout->lineAt(layout->lineCount()-1);
	}
	qreal x = mapFromScene(xCoord, 0).x();
	kDebug() << x;
	int p = line.xToCursor(x);
	cursor.setPosition(p);
    }
    setTextCursor(cursor);
    setFocus();
}

Cantor::Session* WorksheetTextItem::session()
{
    return qobject_cast<Worksheet*>(scene())->session();
}

void WorksheetTextItem::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Left:
	if (textCursor().atStart()) {
	    emit moveToPrevious(BottomRight, 0);
	    kDebug()<<"Reached leftmost valid position";
	    return;
	}
	break;
    case Qt::Key_Right:
	if (textCursor().atEnd()) {
	    emit moveToNext(TopLeft, 0);
	    kDebug()<<"Reached rightmost valid position";
	    return;
	}
	break;
    case Qt::Key_Up:
	if (!textCursor().movePosition(QTextCursor::Up)) {
	    qreal x = mapToScene(localCursorPosition()).x();
	    emit moveToPrevious(BottomCoord, x);
	    kDebug()<<"Reached topmost valid position" << localCursorPosition().x();
	    return;
	}
	break;
    case Qt::Key_Down:
	if (!textCursor().movePosition(QTextCursor::Down)) {
	    qreal x = mapToScene(localCursorPosition()).x();
	    emit moveToNext(TopCoord, x);
	    kDebug()<<"Reached bottommost valid position" << localCursorPosition().x();
	    return;
	}
	break;
    case Qt::Key_Tab:
	emit tabPressed();
	// returning here should probably be optional
	return;
    case Qt::Key_Enter:
    case Qt::Key_Return:
	if (event->modifiers() == Qt::ShiftModifier) {
	    emit execute();
	    return;
	}
	break;
    default:
	break;
    }
    this->QGraphicsTextItem::keyPressEvent(event);
}

void WorksheetTextItem::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event);

    emit receivedFocus(document());
}

#include "worksheettextitem.moc"
