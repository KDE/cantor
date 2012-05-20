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
    m_completionEnabled = false;
    m_completionActive = false;
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
    emit cursorPositionChanged(cursor);
}

QPointF WorksheetTextItem::localCursorPosition() const
{
    QTextCursor cursor = textCursor();
    QTextBlock block = cursor.block();
    int p = cursor.position();
    QTextLine line = block.layout()->lineForTextPosition(p);
    if (!line.isValid()) // this can happen for empty lines
	return block.layout()->position();
    return QPointF(line.cursorToX(p), line.y() + line.height());
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

void WorksheetTextItem::enableCompletion(bool e)
{
    m_completionEnabled = e;
}

void WorksheetTextItem::activateCompletion(bool a)
{
    m_completionActive = a;
}

void WorksheetTextItem::setFocusAt(int pos, qreal xCoord)
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
	    kDebug() << document()->blockCount() << "blocks";
	    kDebug() << document()->lastBlock().lineCount() << "lines in last block";
	    line = layout->lineAt(document()->lastBlock().lineCount()-1);
	}
	qreal x = mapFromScene(xCoord, 0).x();
	kDebug() << x;
	int p = line.xToCursor(x);
	cursor.setPosition(p);
    }
    setTextCursor(cursor);
    emit cursorPositionChanged(cursor);
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
	if (event->modifiers() == Qt::NoModifier && textCursor().atStart()) {
	    emit moveToPrevious(BottomRight, 0);
	    kDebug()<<"Reached leftmost valid position";
	    return;
	}
	break;
    case Qt::Key_Right:
	if (event->modifiers() == Qt::NoModifier && textCursor().atEnd()) {
	    emit moveToNext(TopLeft, 0);
	    kDebug()<<"Reached rightmost valid position";
	    return;
	}
	break;
    case Qt::Key_Up:
	if (event->modifiers() == Qt::NoModifier && !textCursor().movePosition(QTextCursor::Up)) {
	    qreal x = mapToScene(localCursorPosition()).x();
	    emit moveToPrevious(BottomCoord, x);
	    kDebug()<<"Reached topmost valid position" << localCursorPosition().x();
	    return;
	}
	break;
    case Qt::Key_Down:
	if (event->modifiers() == Qt::NoModifier && !textCursor().movePosition(QTextCursor::Down)) {
	    qreal x = mapToScene(localCursorPosition()).x();
	    emit moveToNext(TopCoord, x);
	    kDebug()<<"Reached bottommost valid position" << localCursorPosition().x();
	    return;
	}
	break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
	if (event->modifiers() == Qt::ShiftModifier) {
	    emit execute();
	    return;
	} else if (event->modifiers() == Qt::NoModifier && m_completionActive) {
	    emit applyCompletion();
	    return;
	}
	break;
    default:
	break;
    }
    qreal h = boundingRect().height();
    int p = textCursor().position();
    this->WorksheetStaticTextItem::keyPressEvent(event);
    if (h != boundingRect().height()) {
	updateGeometry();
	emit sizeChanged();
    }
    if (p != textCursor().position())
	emit cursorPositionChanged(textCursor());
}

bool WorksheetTextItem::sceneEvent(QEvent *event)
{
    // QGraphicsTextItem's TabChangesFocus feature prevents calls to
    // keyPressEvent for Tab, even when it's turned off. So we got to catch
    // that here.
    if (event->type() == QEvent::KeyPress) {
	QKeyEvent* kev = dynamic_cast<QKeyEvent*>(event);
	if (kev->key() == Qt::Key_Tab && kev->modifiers() == Qt::NoModifier) {
	    if (!m_completionEnabled)
		insertTab();
	    else
		emit tabPressed();
	    return true;
	} else if ((kev->key() == Qt::Key_Tab && 
		    kev->modifiers() == Qt::ShiftModifier) ||
		   kev->key() == Qt::Key_Backtab) {
	    emit backtabPressed();
	    return true;
	}
    }
    return WorksheetStaticTextItem::sceneEvent(event);
}

void WorksheetTextItem::focusInEvent(QFocusEvent *event)
{
    WorksheetStaticTextItem::focusInEvent(event);
    emit receivedFocus(this);
}

void WorksheetTextItem::focusOutEvent(QFocusEvent *event)
{
    WorksheetStaticTextItem::focusOutEvent(event);
    emit cursorPositionChanged(QTextCursor());
}

void WorksheetTextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    int p = textCursor().position();
    WorksheetStaticTextItem::mousePressEvent(event);
    if (p != textCursor().position())
	emit cursorPositionChanged(textCursor());
}

void WorksheetTextItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)

    emit doubleClick();
}

void WorksheetTextItem::insertTab()
{
    QTextLayout *layout = textCursor().block().layout();
    if (!layout) {
	textCursor().insertText("    ");
    } else {
	QTextLine line = layout->lineAt(textCursor().position());
	int i = textCursor().position() - line.textStart();
	i = ((i+4) & (~3)) - i;
	textCursor().insertText(QString(' ').repeated(i));
    }

    emit cursorPositionChanged(textCursor());
}


#include "worksheettextitem.moc"
