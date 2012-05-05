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

#include "commandentry.h"

#include "lib/expression.h"
#include "lib/result.h"
#include "lib/helpresult.h"
#include "lib/completionobject.h"
#include "lib/syntaxhelpobject.h"
#include "lib/defaulthighlighter.h"
#include "lib/session.h"
#include "worksheet.h"

CommandEntry::CommandEntry() : WorksheetEntry()
{
    m_expression = 0;
    m_completionObject = 0;
    m_syntaxHelpObject = 0;


    QGraphicsLinearLayout *horizontalLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    m_promptItem = new QGraphicsTextItem(this);
    horizontalLayout->addItem(m_promptItem);
    m_verticalLayout = new QGraphicsLinearLayout(Qt::Vertical, horizontalLayout);
    m_commandItem = new WorksheetTextItem(this);
    verticalLayout->addItem(m_commandItem);
    m_errorItem = 0;
    m_resultItem = 0;

    m_promptItem->setOwnedByLayout(false);
    m_commandItem->setOwnedByLayout(false);
    this->setLayout(horizontalLayout);

    m_commandItem.setTextIteractionFlags(Qt::TextEditorInteraction);
    // ToDo: pass information about the desired cursor position.
    connect(m_commandItem, SIGNAL(leftmostValidPositionReached()),
	    worksheet(), SLOT(moveToPreviousEntry()));
    connect(m_commandItem, SIGNAL(rightmostValidPositionReached()),
	    worksheet(), SLOT(moveToNextEntry())); // change!
    connect(m_commandItem, SIGNAL(topmostValidLineReached()),
	    worksheet(), SLOT(moveToPreviousEntry()));
    connect(m_commandItem, SIGNAL(bottommostValidLineReached()),
	    worksheet(), SLOT(moveToNextEntry())); // change!
    connect(m_commandItem, SIGNAL(receivedFocus(QTextDocument*)),
	    worksheet(), SLOT(highlightDocument(QTextDocument*)));

}

CommandEntry::~CommandEntry()
{
    if (m_completionBox)
	m_completionBox->deleteLater();
}

int CommandEntry::type() const
{
    return type;
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
	m_verticalLayout->removeItem(m_errorItem);
        m_errorItem->deleteLater();
	m_errorItem = 0;
    }

    removeResult();

    foreach(QGraphicsSimpleTextItem* item, m_informationItems)
    {
	m_verticalLayout->removeItem(item);
	item->deleteLater();
    }
    m_informationItems.clear();

    // Delete any previous result
    if (m_resultItem)
    {
	m_verticalLayout->removeItem(m_resultItem);
	m_resultItem = 0;
    }

    m_expression=expr;

    connect(expr, SIGNAL(gotResult()), this, SLOT(update()));
    connect(expr, SIGNAL(idChanged()), this, SLOT(updatePrompt()));
    connect(expr, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SLOT(expressionChangedStatus(Cantor::Expression::Status)));
    connect(expr, SIGNAL(needsAdditionalInformation(const QString&)), this, SLOT(showAdditionalInformationPrompt(const QString&)));
    connect(expr, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SLOT(updatePrompt()));

    updatePrompt();

    if(expr->result())
    {
        worksheet()->gotResult(expr);
        update();
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

void CommandEntry::keyPressEvent(QKeyEvent* event)
{
    // ... This function is not called when m_commandItem has the
    // focus. Is there a function that is called before
    // the keyEvent reaches the QGraphicsTextItem?
}




bool CommandEntry::acceptRichText()
{
    return false;
}

void CommandEntry::setContent(const QString& content)
{
    m_commandItem.setText(content);
}

void CommandEntry::setContent(const QDomElement& content, const KZip& file)
{
    m_commandItem.setText(content.firstChildElement("Command").text());

    LoadedExpression* expr=new LoadedExpression( m_worksheet->session() );
    expr->loadFromXml(content, file);

    setExpression(expr);
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

bool CommandEntry::evaluate(bool current)
{
    if (!current)
	return evaluateCommand();
    if (m_commandItem->hasFocus()) {
	return evaluateCommand();
    } else if (informationItemHasFocus()) {
	addInformation();
	return true;
    }
    return false;
}

bool CommandEntry::evaluateCommand()
{
    removeContextHelp();
    QToolTip::hideText();

    QString cmd = command();
    kDebug()<<"evaluating: "<<cmd;

    if(cmd.isEmpty())
        return false;

    Cantor::Expression* expr;
    expr=m_worksheet->session()->evaluateExpression(cmd);
    connect(expr, SIGNAL(gotResult()), m_worksheet, SLOT(gotResult()));

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
    Cantor::Expression *expr = expression();
    if (expr == 0 || expr->type() == 0)
	return;

    if (expr->result()->type() == Cantor::HelpResult::Type)
	return; // Help is handled elsewhere

    if (!m_resultItem) {
	m_resultItem = new QGraphicsTextItem(this);
	m_verticalLayout->addItem(m_resultItem);
    }
    QTextCursor cursor = m_resultItem->textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    worksheet()->resultProxy()->insertResult(cursor, expr->result());
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
    default:
	return;
    }

    if(!m_errorItem)
    {
        int row;
        if(m_resultItem)
            row = m_verticalLayout->count() - 1;
        else
            row = m_verticalLayout->count();
	m_errorItem = new QGraphicsTextItem(this);
	m_verticalLayout->insertItem(row, m_errorItem);
    }

    m_errorItem->setHtml(text);
}

bool CommandEntry::isEmpty()
{
    if (m_commandItem->toPlainText().trimmed().isEmpty()) {
	if (m_resultItem && !m_resultItem->toPlainText().trimmed().isEmpty())
	    return false;
	return true;
    }
    return false;
}

void CommandEntry::setCompletion(Cantor::CompletionObject* tc)
{
    if (m_completionObject)
	removeContextHelp();

    m_completionObject = tc;
    connect(tc, SIGNAL(done()). this, SLOT(showCompletions()));
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

        QToolTip::showText(QPoint(), QString(), worksheet());
	if (m_completionBox)
	    m_completionBox->deleteLater();
	m_completionBox = new KCompletionBox(m_worksheet);
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

	QPointF cursorPos = m_commandItem->cursorPosition();
	cursorPos = mapToScene(popupPoint);
	const QPoint popupPoint = worksheetView()->mapFromScene(cursorPoint);
	m_completionBox->popup();
	m_completionBox->move(worksheetView()->mapToGlobal(popupPoint));
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
    const QString line = currentLine(m_worksheet->textCursor());
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
        QToolTip::showText(QPoint(), QString(), m_worksheet);
	m_completionBox->setItems(m_completionObject->allMatches());
	QList<QListWidgetItem*> items = m_completionBox->findItems(m_completionObject->command(), Qt::MatchFixedString|Qt::MatchCaseSensitive);
	if (!items.empty())
	    m_completionBox->setCurrentItem(items.first());

	QPointF cursorPos = m_commandItem->cursorPosition();
	cursorPos = mapToScene(popupPoint);
	const QPoint popupPoint = worksheetView()->mapFromScene(cursorPoint);
	//m_completionBox->popup();
	m_completionBox->move(worksheetView()->mapToGlobal(popupPoint));
    } else
    {
        removeContextHelp();
    }
}

void CommandEntry::completeCommandTo(const QString& completion, CompletionMode mode)
{
    kDebug() << "completion: " << completion;

    if (mode == FinalCompletion) {
        Cantor::SyntaxHelpObject* obj = m_worksheet->session()->syntaxHelpFor(completion);
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
    QPointF cursorPos = m_commandItem->cursorPosition();
    cursorPos = mapToScene(popupPoint);
    const QPoint popupPoint = worksheetView()->mapFromScene(cursorPoint);
    const QPoint pos = worksheetView()->mapToGlobal(popupPoint);

    QToolTip::showText(pos, msg, m_worksheet);
}

void CommandEntry::resultDeleted()
{
    kDebug()<<"result got removed...";
}

void CommandEntry::addInformation()
{
    QString inf = m_informationItems.last()->toPlainText();
    inf.replace(QChar::ParagraphSeparator, '\n');
    inf.replace(QChar::LineSeparator, '\n');

    kDebug()<<"adding information: "<<inf;
    if(m_expression)
        m_expression->addInformation(inf);
}

void CommandEntry::showAdditionalInformationPrompt(const QString& question)
{
    int row;
    if (actualInformationCell().isValid())
        row = m_informationItems.size() + 1;
    else
        row = 1;

    QGraphicsSimpleTextItem* questionItem = new QGraphicsSimpleTextItem(this);
    WorksheetTextItem* answerItem = new WorksheetTextItem(this);
    questionItem->setText(question);
    m_verticalLayout->addItem(questionItem);
    m_verticalLayout->addItem(answerItem);
    m_informationItems.append(answerItem);

    answerItem->setFocus();
}

void CommandEntry::removeResult()
{
    if (m_resultItem) {
	m_verticalLayout->removeItem(m_resultItem);
	m_resultItem->deleteLater();
	m_resultItem = 0;
    }

    if(m_expression)
    {
        m_expression->clearResult();
    }

}

void CommandEntry::removeContextHelp()
{
    disconnect(worksheet(), SIGNAL(textChanged()), this,
	       SLOT(completedLineChanged()));
    if(m_completionObject)
        m_completionObject->deleteLater();

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
        if(m_expression ->status() == Cantor::Expression::Computing&& m_worksheet->isRunning())
            cformat.setForeground(color.foreground(KColorScheme::PositiveText));
        else if(m_expression ->status() == Cantor::Expression::Error)
            cformat.setForeground(color.foreground(KColorScheme::NegativeText));
        else if(m_expression ->status() == Cantor::Expression::Interrupted)
            cformat.setForeground(color.foreground(KColorScheme::NeutralText));
        else
            cformat.setFontWeight(QFont::Normal);
    }

    c.insertText(CommandEntry::Prompt,cformat);
}

WorksheetView* CommandEntry::worksheetView()
{
    return worksheet()->worksheetView();
}

