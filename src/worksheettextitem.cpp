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

#include "worksheettextitem.h"
#include "worksheet.h"

#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextLine>
#include <QGraphicsSceneResizeEvent>

#include <kdebug.h>

WorksheetTextItem::WorksheetTextItem(QGraphicsWidget* parent, QGraphicsLayoutItem* lparent) 
    : WorksheetStaticTextItem(parent, lparent)
{
    setTextInteractionFlags(Qt::TextEditorInteraction);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
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
    if (!line.isValid()) // this can happen for empty lines
	return block.layout()->position();
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
	    kDebug() << document()->lastBlock().lineCount() << "lines";
	    line = layout->lineAt(document()->lastBlock().lineCount()-1);
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
    qreal h = boundingRect().height();
    this->QGraphicsTextItem::keyPressEvent(event);
    if (h != boundingRect().height()) {
	updateGeometry();
	emit sizeChanged();
    }
}

void WorksheetTextItem::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event);

    emit receivedFocus(document());
}

#include "worksheettextitem.moc"
