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
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
    Copyright (C) 2012 Martin Kuettler <martin.kuettler@gmail.com>
 */

#include "commandentry.h"
#include "worksheet.h"
#include "worksheettextitem.h"
#include "resultitem.h"
#include "loadedexpression.h"
#include "settings.h"
#include "lib/expression.h"
#include "lib/result.h"
#include "lib/helpresult.h"
#include "lib/completionobject.h"
#include "lib/syntaxhelpobject.h"
#include "lib/defaulthighlighter.h"
#include "lib/session.h"

#include <QTextDocument>
#include <QTextCursor>
#include <QTextLine>
#include <QToolTip>
#include <QtGlobal>

#include <kdebug.h>
#include <klocale.h>
#include <KColorScheme>

const QString CommandEntry::Prompt=">>> ";
const double CommandEntry::HorizontalSpacing = 4;
const double CommandEntry::VerticalSpacing = 4;

CommandEntry::CommandEntry(Worksheet* worksheet) : WorksheetEntry(worksheet)
{
    m_expression = 0;
    m_completionObject = 0;
    m_syntaxHelpObject = 0;

    m_promptItem = new WorksheetTextItem(this, Qt::NoTextInteraction);
    m_promptItem->setPlainText(Prompt);
    m_commandItem = new WorksheetTextItem(this, Qt::TextEditorInteraction);
    m_commandItem->enableCompletion(true);
    m_errorItem = 0;
    m_resultItem = 0;

    connect(m_commandItem, SIGNAL(tabPressed()), this, SLOT(showCompletion()));
    connect(m_commandItem, SIGNAL(backtabPressed()),
	    this, SLOT(selectPreviousCompletion()));
    connect(m_commandItem, SIGNAL(applyCompletion()),
	    this, SLOT(applySelectedCompletion()));
    connect(m_commandItem, SIGNAL(execute()), this, SLOT(evaluateCommand()));
    connect(m_commandItem, SIGNAL(moveToPrevious(int, qreal)),
	    this, SLOT(moveToPreviousItem(int, qreal)));
    connect(m_commandItem, SIGNAL(moveToNext(int, qreal)),
	    this, SLOT(moveToNextItem(int, qreal)));
    connect(m_commandItem, SIGNAL(receivedFocus(WorksheetTextItem*)),
	    worksheet, SLOT(highlightItem(WorksheetTextItem*)));
}

CommandEntry::~CommandEntry()
{
    if (m_completionBox)
	m_completionBox->deleteLater();
}

int CommandEntry::type() const
{
    return Type;
}

void CommandEntry::populateMenu(KMenu *menu, const QPointF& pos)
{
    kDebug() << "populate Menu";
    WorksheetEntry::populateMenu(menu, pos);
}

void CommandEntry::moveToNextItem(int pos, qreal x)
{
    WorksheetTextItem* item = qobject_cast<WorksheetTextItem*>(sender());

    if (!item)
	return;

    if (item == m_commandItem || item == 0) {
	if (m_informationItems.isEmpty())
	    moveToNextEntry(pos, x);
	else
	    currentInformationItem()->setFocusAt(pos, x);
    } else if (item == currentInformationItem()) {
	moveToNextEntry(pos, x);
    }
}

void CommandEntry::moveToPreviousItem(int pos, qreal x)
{
    WorksheetTextItem* item = qobject_cast<WorksheetTextItem*>(sender());

    if (!item)
	return;

    if (item == m_commandItem || item == 0) {
	moveToPreviousEntry(pos, x);
    } else if (item == currentInformationItem()) {
	m_commandItem->setFocusAt(pos, x);
    }
}

QString CommandEntry::command()
{
    QString cmd = m_commandItem->toPlainText();
    cmd.replace(QChar::ParagraphSeparator, '\n'); //Replace the U+2029 paragraph break by a Normal Newline
    cmd.replace(QChar::LineSeparator, '\n'); //Replace the line break by a Normal Newline
    return cmd;
}

void CommandEntry::setExpression(Cantor::Expression* expr)
{
    if ( m_expression )
        m_expression->deleteLater();

    // Delete any previus error and/or result
    if(m_errorItem)
    {
        m_errorItem->deleteLater();
	m_errorItem = 0;
    }

    foreach(WorksheetTextItem* item, m_informationItems)
    {
	item->deleteLater();
    }
    m_informationItems.clear();

    m_expression = 0;
    // Delete any previous result
    removeResult();

    m_expression=expr;

    connect(expr, SIGNAL(gotResult()), this, SLOT(updateEntry()));
    connect(expr, SIGNAL(idChanged()), this, SLOT(updatePrompt()));
    connect(expr, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SLOT(expressionChangedStatus(Cantor::Expression::Status)));
    connect(expr, SIGNAL(needsAdditionalInformation(const QString&)), this, SLOT(showAdditionalInformationPrompt(const QString&)));
    connect(expr, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SLOT(updatePrompt()));

    updatePrompt();

    if(expr->result())
    {
        worksheet()->gotResult(expr);
        updateEntry();
    }
    if(expr->status()!=Cantor::Expression::Computing)
    {
        expressionChangedStatus(expr->status());
    }
}

Cantor::Expression* CommandEntry::expression()
{
    return m_expression;
}



bool CommandEntry::acceptRichText()
{
    return false;
}

void CommandEntry::setContent(const QString& content)
{
    m_commandItem->setPlainText(content);
}

void CommandEntry::setContent(const QDomElement& content, const KZip& file)
{
    m_commandItem->setPlainText(content.firstChildElement("Command").text());

    LoadedExpression* expr=new LoadedExpression( worksheet()->session() );
    expr->loadFromXml(content, file);

    setExpression(expr);
}

void CommandEntry::showCompletion()
{
    //get the current line of the entry. If it's empty, ignore the call,
    //otherwise check for tab completion (if supported by the backend)
    const QString line = currentLine();

    if(line.trimmed().isEmpty())
    {
        return;
    } else if (isShowingCompletionPopup()) {
	QString comp = m_completionObject->completion();
	kDebug() << "command" << m_completionObject->command();
	kDebug() << "completion" << comp;
	if (comp != m_completionObject->command()
	    || !m_completionObject->hasMultipleMatches()) {
	    if (m_completionObject->hasMultipleMatches()) {
		completeCommandTo(comp, PreliminaryCompletion);
	    } else {
		completeCommandTo(comp, FinalCompletion);
		m_completionBox->hide();
	    }
	} else {
	    m_completionBox->down();
	}
    } else {
	int p = m_commandItem->textCursor().positionInBlock();
        Cantor::CompletionObject* tco=worksheet()->session()->completionFor(line, p);
        if(tco)
            setCompletion(tco);
    }
}

void CommandEntry::selectPreviousCompletion()
{
    if (isShowingCompletionPopup())
	m_completionBox->up();
}

QString CommandEntry::toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq)
{
    Q_UNUSED(commentStartingSeq);
    Q_UNUSED(commentEndingSeq);

    if (command().isEmpty())
        return QString();
    return command() + commandSep;
}

QDomElement CommandEntry::toXml(QDomDocument& doc, KZip* archive)
{
    if (expression())
    {
        if ( archive )
            expression()->saveAdditionalData( archive );
        return expression()->toXml(doc);
    }
    QDomElement expr=doc.createElement( "Expression" );
    QDomElement cmd=doc.createElement( "Command" );
    QDomText cmdText=doc.createTextNode( command() );
    cmd.appendChild( cmdText );
    expr.appendChild( cmd );
    return expr;
}

QString CommandEntry::currentLine()
{
    if (!m_commandItem->hasFocus())
	return QString();

    QTextBlock block = m_commandItem->textCursor().block();
    return block.text();
}

bool CommandEntry::evaluate(int evalOp)
{
    bool success = false;

    if (!(evalOp & FocusedItemOnly) || m_commandItem->hasFocus()) {
	success = evaluateCommand(evalOp);
    } else if (informationItemHasFocus()) {
	addInformation();
	success = true;
    }

    if (evalOp & EvaluateNextEntries || Settings::self()->autoEval())
	m_evaluationFlag = EvaluateNextEntries;
    else
	m_evaluationFlag = 0;

    return success;
}

bool CommandEntry::evaluateCommand(int evalOp)
{
    removeContextHelp();
    QToolTip::hideText();

    QString cmd = command();
    kDebug()<<"evaluating: "<<cmd;

    if (evalOp & EvaluateNextEntries || Settings::self()->autoEval())
	m_evaluationFlag = EvaluateNextEntries;
    else
	m_evaluationFlag = 0;

    if(cmd.isEmpty()) {
	removeResult();
	foreach(WorksheetTextItem* item, m_informationItems) {
	    item->deleteLater();
	}
	m_informationItems.clear();
	recalculateSize();

	evaluateNext(m_evaluationFlag);
        return false;
    }

    Cantor::Expression* expr;
    expr = worksheet()->session()->evaluateExpression(cmd);
    connect(expr, SIGNAL(gotResult()), worksheet(), SLOT(gotResult()));

    setExpression(expr);

    return true;
}

void CommandEntry::interruptEvaluation()
{
    Cantor::Expression *expr = expression();
    if(expr)
        expr->interrupt();
}

void CommandEntry::updateEntry()
{
    kDebug() << "update Entry";
    Cantor::Expression *expr = expression();
    if (expr == 0 || expr->result() == 0)
	return;

    if (expr->result()->type() == Cantor::HelpResult::Type)
	return; // Help is handled elsewhere

    if (!m_resultItem) {
	m_resultItem = ResultItem::create(this, expr->result());
	kDebug() << "new result";
	animateSizeChange();
    } else {
	m_resultItem = m_resultItem->updateFromResult(expr->result());
	kDebug() << "update result";
	animateSizeChange();
    }
}

void CommandEntry::expressionChangedStatus(Cantor::Expression::Status status)
{
    QString text;
    switch (status)
    {
    case Cantor::Expression::Error:
	text = m_expression->errorMessage();
	break;
    case Cantor::Expression::Interrupted:
	text = i18n("Interrupted");
	break;
    case Cantor::Expression::Done:
	evaluateNext(m_evaluationFlag);
	return;
    default:
	return;
    }

    m_commandItem->setFocusAt(WorksheetTextItem::BottomRight, 0);

    if(!m_errorItem)
    {
	m_errorItem = new WorksheetTextItem(this, Qt::TextSelectableByMouse);
    }

    m_errorItem->setHtml(text);
    recalculateSize();
}

bool CommandEntry::isEmpty()
{
    if (m_commandItem->toPlainText().trimmed().isEmpty()) {
	if (m_resultItem)
	    return false;
	return true;
    }
    return false;
}

bool CommandEntry::focusEntry(int pos, qreal xCoord)
{
    if (aboutToBeRemoved())
	return false;
    WorksheetTextItem* item;
    if (pos == WorksheetTextItem::TopLeft || pos == WorksheetTextItem::TopCoord)
	item = m_commandItem;
    else if (m_informationItems.size() && currentInformationItem()->isEditable())
	item = currentInformationItem();
    else
	item = m_commandItem;

    item->setFocusAt(pos, xCoord);
    return true;
}

void CommandEntry::setCompletion(Cantor::CompletionObject* tc)
{
    if (m_completionObject)
	removeContextHelp();

    m_completionObject = tc;
    connect(tc, SIGNAL(done()), this, SLOT(showCompletions()));
    connect(tc, SIGNAL(lineDone(QString, int)), this, SLOT(completeLineTo(QString, int)));
}

void CommandEntry::showCompletions()
{
    disconnect(m_completionObject, SIGNAL(done()), this, SLOT(showCompletions()));
    QString completion = m_completionObject->completion();
    kDebug()<<"completion: "<<completion;
    kDebug()<<"showing "<<m_completionObject->allMatches();

    if(m_completionObject->hasMultipleMatches())
    {
	completeCommandTo(completion);

        QToolTip::showText(QPoint(), QString(), worksheetView());
	if (m_completionBox)
	    m_completionBox->deleteLater();
	m_completionBox = new KCompletionBox(worksheetView());
	m_completionBox->setItems(m_completionObject->allMatches());
	QList<QListWidgetItem*> items = m_completionBox->findItems(m_completionObject->command(), Qt::MatchFixedString|Qt::MatchCaseSensitive);
	if (!items.empty())
	    m_completionBox->setCurrentItem(items.first());
	m_completionBox->setTabHandling(false);
	m_completionBox->setActivateOnSelect(true);
	connect(m_completionBox, SIGNAL(activated(const QString&)), this,
		SLOT(applySelectedCompletion()));
	connect(m_commandItem->document(), SIGNAL(contentsChanged()), this,
		SLOT(completedLineChanged()));
	connect(m_completionObject, SIGNAL(done()), this, SLOT(updateCompletions()));

	m_commandItem->activateCompletion(true);
	QPointF cursorPos = m_commandItem->cursorPosition();
	m_completionBox->popup();
	m_completionBox->move(toGlobalPosition(cursorPos));
    } else
    {
	completeCommandTo(completion, FinalCompletion);
    }
}

bool CommandEntry::isShowingCompletionPopup()
{

    return m_completionBox && m_completionBox->isVisible();
}

void CommandEntry::applySelectedCompletion()
{
    QListWidgetItem* item = m_completionBox->currentItem();
    if(item)
	completeCommandTo(item->text(), FinalCompletion);
    m_completionBox->hide();
}

void CommandEntry::completedLineChanged()
{
    if (!isShowingCompletionPopup()) {
	// the completion popup is not visible anymore, so let's clean up
	removeContextHelp();
	return;
    }
    const QString line = currentLine();
    m_completionObject->updateLine(line, m_commandItem->textCursor().positionInBlock());
}

void CommandEntry::updateCompletions()
{
    if (!m_completionObject)
	return;
    QString completion = m_completionObject->completion();
    kDebug()<<"completion: "<<completion;
    kDebug()<<"showing "<<m_completionObject->allMatches();

    if(m_completionObject->hasMultipleMatches() || !completion.isEmpty())
    {
        QToolTip::showText(QPoint(), QString(), worksheetView());
	m_completionBox->setItems(m_completionObject->allMatches());
	QList<QListWidgetItem*> items = m_completionBox->findItems(m_completionObject->command(), Qt::MatchFixedString|Qt::MatchCaseSensitive);
	if (!items.empty())
	    m_completionBox->setCurrentItem(items.first());

	QPointF cursorPos = m_commandItem->cursorPosition();
	m_completionBox->move(toGlobalPosition(cursorPos));
    } else
    {
        removeContextHelp();
    }
}

void CommandEntry::completeCommandTo(const QString& completion, CompletionMode mode)
{
    kDebug() << "completion: " << completion;

    if (mode == FinalCompletion) {
        Cantor::SyntaxHelpObject* obj = worksheet()->session()->syntaxHelpFor(completion);
        if(obj)
            setSyntaxHelp(obj);
    } else {
	if(m_syntaxHelpObject)
	    m_syntaxHelpObject->deleteLater();
	m_syntaxHelpObject=0;
    }

    Cantor::CompletionObject::LineCompletionMode cmode;
    if (mode == PreliminaryCompletion)
	cmode = Cantor::CompletionObject::PreliminaryCompletion;
    else
	cmode = Cantor::CompletionObject::FinalCompletion;
    m_completionObject->completeLine(completion, cmode);
}

void CommandEntry::completeLineTo(const QString& line, int index)
{
    kDebug() << "line completion: " << line;
    QTextCursor cursor = m_commandItem->textCursor();
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    int startPosition = cursor.position();
    cursor.insertText(line);
    cursor.setPosition(startPosition + index);
    m_commandItem->setTextCursor(cursor);

    if (m_syntaxHelpObject) {
	m_syntaxHelpObject->fetchSyntaxHelp();
	// If we are about to show syntax help, then this was the final
	// completion, and we should clean up.
	removeContextHelp();
    }
}

void CommandEntry::setSyntaxHelp(Cantor::SyntaxHelpObject* sh)
{
    if(m_syntaxHelpObject)
        m_syntaxHelpObject->deleteLater();

    m_syntaxHelpObject=sh;
    connect(sh, SIGNAL(done()), this, SLOT(showSyntaxHelp()));
}

void CommandEntry::showSyntaxHelp()
{
    const QString& msg = m_syntaxHelpObject->toHtml();
    const QPointF cursorPos = m_commandItem->cursorPosition();

    QToolTip::showText(toGlobalPosition(cursorPos), msg, worksheetView());
}

void CommandEntry::resultDeleted()
{
    kDebug()<<"result got removed...";
}

void CommandEntry::addInformation()
{
    WorksheetTextItem *answerItem = currentInformationItem();
    answerItem->setTextInteractionFlags(Qt::TextSelectableByMouse);

    QString inf = answerItem->toPlainText();
    inf.replace(QChar::ParagraphSeparator, '\n');
    inf.replace(QChar::LineSeparator, '\n');

    kDebug()<<"adding information: "<<inf;
    if(m_expression)
        m_expression->addInformation(inf);
}

void CommandEntry::showAdditionalInformationPrompt(const QString& question)
{
    WorksheetTextItem* questionItem = new WorksheetTextItem(this, Qt::TextSelectableByMouse);
    WorksheetTextItem* answerItem = new WorksheetTextItem(this, Qt::TextEditorInteraction);
    questionItem->setPlainText(question);
    m_informationItems.append(questionItem);
    m_informationItems.append(answerItem);
    connect(answerItem, SIGNAL(moveToPrevious(int, qreal)),
	    this, SLOT(moveToPreviousItem(int, qreal)));
    connect(answerItem, SIGNAL(moveToNext(int, qreal)),
	    this, SLOT(moveToNextItem(int, qreal)));

    connect(answerItem, SIGNAL(execute()), this, SLOT(addInformation()));
    answerItem->setFocus();
    recalculateSize();
}

void CommandEntry::removeResult()
{
    if(m_expression)
    {
        m_expression->clearResult();
    }

    if (m_resultItem) {
	QGraphicsObject* obj = m_resultItem->graphicsObject();
	m_resultItem = 0;
	fadeOutItem(obj);
    }
}

void CommandEntry::removeContextHelp()
{
    disconnect(m_commandItem->document(), SIGNAL(contentsChanged()), this,
	       SLOT(completedLineChanged()));
    if(m_completionObject)
        m_completionObject->deleteLater();

    m_commandItem->activateCompletion(false);
    m_completionObject = 0;
    if (m_completionBox)
	m_completionBox->hide();
}


void CommandEntry::updatePrompt()
{
    KColorScheme color = KColorScheme( QPalette::Normal, KColorScheme::View);

    m_promptItem->setPlainText("");
    QTextCursor c = m_promptItem->textCursor();
    QTextCharFormat cformat = c.charFormat();

    cformat.clearForeground();
    c.setCharFormat(cformat);
    cformat.setFontWeight(QFont::Bold);

    //insert the session id if available
    if(m_expression && worksheet()->showExpressionIds())
        c.insertText(QString::number(m_expression->id()),cformat);

    //detect the correct color for the prompt, depending on the
    //Expression state
    if(m_expression)
    {
        if(m_expression ->status() == Cantor::Expression::Computing&&worksheet()->isRunning())
            cformat.setForeground(color.foreground(KColorScheme::PositiveText));
        else if(m_expression ->status() == Cantor::Expression::Error)
            cformat.setForeground(color.foreground(KColorScheme::NegativeText));
        else if(m_expression ->status() == Cantor::Expression::Interrupted)
            cformat.setForeground(color.foreground(KColorScheme::NeutralText));
        else
            cformat.setFontWeight(QFont::Normal);
    }

    c.insertText(CommandEntry::Prompt,cformat);
    recalculateSize();
}

WorksheetTextItem* CommandEntry::currentInformationItem()
{
    if (m_informationItems.isEmpty())
	return 0;
    return m_informationItems.last();
}

WorksheetView* CommandEntry::worksheetView()
{
    return worksheet()->worksheetView();
}

bool CommandEntry::informationItemHasFocus()
{
    if (m_informationItems.isEmpty())
	return false;
    return m_informationItems.last()->hasFocus();
}

bool CommandEntry::focusWithinThisItem()
{
    return focusItem() != 0;
}

void CommandEntry::invalidate()
{
    kDebug() << "ToDo: Invalidate here";
}

bool CommandEntry::wantToEvaluate()
{
    return !isEmpty();
}

QPoint CommandEntry::toGlobalPosition(const QPointF& localPos)
{
    const QPointF scenePos = mapToScene(localPos);
    const QPoint viewportPos = worksheetView()->mapFromScene(scenePos);
    return worksheetView()->viewport()->mapToGlobal(viewportPos);
}

void CommandEntry::layOutForWidth(double w, bool force)
{
    if (w == size().width() && !force)
	return;

    m_promptItem->setPos(0,0);
    double x = 0 + m_promptItem->width() + HorizontalSpacing;
    double y = 0;

    m_commandItem->setPos(x,y);
    m_commandItem->setTextWidth(w-x);

    y += qMax(m_commandItem->height(), m_promptItem->height());
    foreach(WorksheetTextItem* information, m_informationItems) {
	y += VerticalSpacing;
	information->setPos(x,y);
	information->setTextWidth(w-x);
	y += information->height();
    }

    if (m_errorItem) {
	y += VerticalSpacing;
	m_errorItem->setPos(x,y);
	m_errorItem->setTextWidth(w-x);
	y += m_errorItem->height();
    }

    if (m_resultItem) {
	y += VerticalSpacing;
	y += m_resultItem->setGeometry(x, y, w-x);
    }
    y += VerticalMargin;

    QSizeF s(w, y);
    if (animationActive()) {
	updateAnimation(s);
    } else {
	setSize(s);
    }
}

