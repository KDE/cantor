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
#include "worksheetentry.h"
#include "epsrenderer.h"

#include <QApplication>
#include <QClipboard>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextLine>
#include <QGraphicsSceneResizeEvent>

#include <kdebug.h>
#include <kglobalsettings.h>
#include <KStandardAction>
#include <KAction>

WorksheetTextItem::WorksheetTextItem(QGraphicsObject* parent, Qt::TextInteractionFlags ti)
    : QGraphicsTextItem(parent)
{
    setTextInteractionFlags(ti);
    if (ti & Qt::TextEditable)
	connect(this, SIGNAL(sizeChanged()), parent,
		SLOT(recalculateSize()));
    m_completionEnabled = false;
    m_completionActive = false;
    setFont(KGlobalSettings::fixedFont());
    connect(document(), SIGNAL(contentsChange(int, int, int)),
	    this, SLOT(setHeight()));
    connect(document(), SIGNAL(contentsChanged()),
	    this, SLOT(testHeight()));
    connect(this, SIGNAL(menuCreated(KMenu*, const QPointF&)), parent,
	    SLOT(populateMenu(KMenu*, const QPointF&)), Qt::DirectConnection);
}

WorksheetTextItem::~WorksheetTextItem()
{
}

int WorksheetTextItem::type() const
{
    return Type;
}

void WorksheetTextItem::setHeight()
{
    m_height = height();
}

void WorksheetTextItem::testHeight()
{
    kDebug() << m_height << height();
    if (m_height != height())
	emit sizeChanged();
}

void WorksheetTextItem::populateMenu(KMenu *menu, const QPointF& pos)
{
    kDebug() << "populate Menu";
    KAction* cut = KStandardAction::cut(this, SLOT(cut()), menu);
    KAction* copy = KStandardAction::copy(this, SLOT(copy()), menu);
    KAction* paste = KStandardAction::paste(this, SLOT(paste()), menu);
    if (!textCursor().hasSelection()) {
	cut->setEnabled(false);
	copy->setEnabled(false);
    }
    if (QApplication::clipboard()->text().isEmpty()) {
	paste->setEnabled(false);
    }
    if (isEditable())
	menu->addAction(cut);
    menu->addAction(copy);
    if (isEditable())
	menu->addAction(paste);
    menu->addSeparator();

    emit menuCreated(menu, mapToParent(pos));
}

void WorksheetTextItem::cut()
{
    copy();
    textCursor().removeSelectedText();
}

void WorksheetTextItem::paste()
{
    textCursor().insertText(QApplication::clipboard()->text());
}

void WorksheetTextItem::copy()
{
    if (!textCursor().hasSelection())
        return;
    QApplication::clipboard()->setText(resolveImages(textCursor()));
}

QString WorksheetTextItem::resolveImages(const QTextCursor& cursor)
{
    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();

    const QString repl = QString(QChar::ObjectReplacementCharacter);
    QString result;
    QTextCursor cursor1 = textCursor();
    cursor1.setPosition(start);
    QTextCursor cursor2 = document()->find(repl, cursor1);

    for (; !cursor2.isNull() && cursor2.selectionEnd() <= end;
	 cursor2 = document()->find(repl, cursor1)) {
	cursor1.setPosition(cursor2.selectionStart(), QTextCursor::KeepAnchor);
	result += cursor1.selectedText();
	QVariant var = cursor2.charFormat().property(EpsRenderer::Delimiter);
	QString delim;
	if (var.isValid())
	    delim = qVariantValue<QString>(var);
	else
	    delim = "";
	result += delim + qVariantValue<QString>(cursor2.charFormat().property(EpsRenderer::Code)) + delim;
	cursor1.setPosition(cursor2.selectionEnd());
    }

    cursor1.setPosition(end, QTextCursor::KeepAnchor);
    result += cursor1.selectedText();
    return result;
}

void WorksheetTextItem::setCursorPosition(const QPointF& pos)
{
    QTextCursor cursor = cursorForPosition(pos);
    setTextCursor(cursor);
    emit cursorPositionChanged(cursor);
    //setLocalCursorPosition(mapFromParent(pos));
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
    int p = cursor.position() - block.position();
    QTextLine line = block.layout()->lineForTextPosition(p);
    if (!line.isValid()) // can this happen?
	return block.layout()->position();
    return QPointF(line.cursorToX(p), line.y() + line.height());
}

QTextCursor WorksheetTextItem::cursorForPosition(const QPointF& pos) const
{
    QPointF lpos = mapFromParent(pos);
    int p = document()->documentLayout()->hitTest(lpos, Qt::FuzzyHit);
    QTextCursor cursor = textCursor();
    cursor.setPosition(p);
    return cursor;
}

/*
void WorksheetTextItem::setEditable(bool e)
{
    if (e)
	setTextInteractionFlags(Qt::TextEditorInteraction);
    else
	setTextInteractionFlags(Qt::TextSelectableByMouse);
}
*/

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
	int p = line.xToCursor(x);
	cursor.setPosition(p);
	// Hack: The code for selecting the last line above does not work.
	// This is a workaround
	if (pos == BottomCoord)
	    while (cursor.movePosition(QTextCursor::Down))
		;
    }
    setTextCursor(cursor);
    emit cursorPositionChanged(cursor);
    setFocus();
}

Cantor::Session* WorksheetTextItem::session()
{
    return worksheet()->session();
}

void WorksheetTextItem::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_C && event->modifiers() == Qt::ControlModifier)
    {
	copy();
	return;
    }

    if (!isEditable())
	return;

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
	} else if (event->modifiers() == Qt::ControlModifier) {
	    emit appendCommandEntry();
	    return;
	} else if (event->modifiers() == Qt::NoModifier && m_completionActive) {
	    emit applyCompletion();
	    return;
	}
	break;
    case Qt::Key_Delete:
	if (event->modifiers() == Qt::ShiftModifier) {
	    emit deleteEntry();
	    return;
	}
	break;
	/* call our custom functions for cut and paste */
    case Qt::Key_X:
	if (event->modifiers() == Qt::ControlModifier) {
	    cut();
	    return;
	}
	break;
    case Qt::Key_V:
	if (event->modifiers() == Qt::ControlModifier) {
	    paste();
	    return;
	}
	break;
    default:
	break;
    }
    int p = textCursor().position();
    QGraphicsTextItem::keyPressEvent(event);
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
	    QTextCursor cursor = textCursor();
	    // maybe we can do something smart with selections here,
	    // but for now we just ignore them.
	    cursor.clearSelection();
	    cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
	    QString sel = cursor.selectedText();
	    bool spacesOnly = true;
	    for (QString::iterator it = sel.begin(); it != sel.end(); ++it) {
		if (*it != ' ') {
		    spacesOnly = false;
		    break;
		}
	    }

	    if (spacesOnly) {
		cursor.setPosition(cursor.selectionEnd());
		while (document()->characterAt(cursor.position()) == ' ')
		    cursor.movePosition(QTextCursor::NextCharacter);
		setTextCursor(cursor);
		insertTab();
	    } else if (m_completionEnabled) {
		emit tabPressed();
	    }
	    return true;
	} else if ((kev->key() == Qt::Key_Tab &&
		    kev->modifiers() == Qt::ShiftModifier) ||
		   kev->key() == Qt::Key_Backtab) {
	    emit backtabPressed();
	    return true;
	}
    }
    return QGraphicsTextItem::sceneEvent(event);
}

void WorksheetTextItem::focusInEvent(QFocusEvent *event)
{
    QGraphicsTextItem::focusInEvent(event);
    parentItem()->ensureVisible();
    emit receivedFocus(this);
}

void WorksheetTextItem::focusOutEvent(QFocusEvent *event)
{
    if (event->reason() == Qt::MouseFocusReason ||
	event->reason() == Qt::OtherFocusReason) {
	QTextCursor cursor = textCursor();
	cursor.clearSelection();
	setTextCursor(cursor);
    }
    QGraphicsTextItem::focusOutEvent(event);
    emit cursorPositionChanged(QTextCursor());
}

void WorksheetTextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    int p = textCursor().position();
    QGraphicsTextItem::mousePressEvent(event);
    if (p != textCursor().position())
	emit cursorPositionChanged(textCursor());
}

void WorksheetTextItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QTextCursor cursor = textCursor();
    const QChar repl = QChar::ObjectReplacementCharacter;

    if (!cursor.hasSelection()) {
	// We look at the current cursor and the next cursor for a
	// ObjectReplacementCharacter
	for (int i = 2; i; --i) {
	    if (document()->characterAt(cursor.position()-1) == repl) {
		setTextCursor(cursor);
		emit doubleClick();
		return;
	    }
	    cursor.movePosition(QTextCursor::NextCharacter);
	}
    } else if (cursor.selectedText().contains(repl)) {
	emit doubleClick();
	return;
    }

    QGraphicsTextItem::mouseDoubleClickEvent(event);
}

void WorksheetTextItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    KMenu *menu = worksheet()->createContextMenu();
    populateMenu(menu, event->pos());

    menu->popup(event->screenPos());
}

void WorksheetTextItem::insertTab()
{
    QTextLayout *layout = textCursor().block().layout();
    QTextCursor cursor = textCursor();
    if (!layout) {
	cursor.insertText("    ");
    } else {
	cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
	int i = cursor.selectionEnd() - cursor.selectionStart();
	i = ((i+4) & (~3)) - i;
	cursor.setPosition(cursor.selectionEnd());
	cursor.insertText(QString(' ').repeated(i));
    }
    // without this line subsequent cursor movement up or down uses the old
    // position
    setTextCursor(cursor);
    emit cursorPositionChanged(textCursor());
}

double WorksheetTextItem::width()
{
    return document()->size().width();
}

double WorksheetTextItem::height()
{
    return document()->size().height();
}

Worksheet* WorksheetTextItem::worksheet()
{
    return qobject_cast<Worksheet*>(scene());
}

#include "worksheettextitem.moc"
