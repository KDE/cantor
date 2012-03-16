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
 */

#include "commandentry.h"
#include "worksheetentry.h"

#include "lib/expression.h"
#include "lib/result.h"
#include "lib/helpresult.h"
#include "lib/completionobject.h"
#include "lib/syntaxhelpobject.h"
#include "lib/defaulthighlighter.h"
#include "lib/session.h"
#include "worksheet.h"
#include "resultproxy.h"
#include "resultcontextmenu.h"
#include "settings.h"
#include "loadedexpression.h"

#include <QEvent>
#include <QKeyEvent>
#include <QTimer>
#include <QTextDocument>
#include <QTextFrame>
#include <QToolTip>
#include <kdebug.h>
#include <kglobal.h>
#include <kcolorscheme.h>
#include <kcompletionbox.h>
#include <klocale.h>
#include <kstandardaction.h>
#include <kaction.h>

const QString CommandEntry::Prompt=">>> ";

CommandEntry::CommandEntry( QTextCursor position, Worksheet* parent ) : WorksheetEntry( position, parent )
{
    m_expression=0;
    m_completionObject=0;
    m_syntaxHelpObject=0;

    connect(m_worksheet, SIGNAL(updatePrompt()), this, SLOT(updatePrompt()));

    QTextTableFormat tableFormat;
    QVector<QTextLength> constraints;
    QFontMetrics metrics(parent->document()->defaultFont());
    constraints<< QTextLength(QTextLength::FixedLength, metrics.width(CommandEntry::Prompt))
               <<QTextLength(QTextLength::PercentageLength, 100);

    tableFormat.setColumnWidthConstraints(constraints);
    tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    tableFormat.setCellSpacing(10);
    tableFormat.setTopMargin(5);

    position=m_frame->firstCursorPosition();

    m_table=position.insertTable(1, 2, tableFormat);
    //make sure, everything is invalid, when the table gets removed
    connect(m_table, SIGNAL(destroyed(QObject*)), this, SLOT(invalidate()));
    //delete the worksheet entry, when the table gets removed from the worksheet
    connect(m_table, SIGNAL(destroyed(QObject*)), this, SLOT(deleteLater()));

    m_table->cellAt(0, 0).firstCursorPosition().insertText(Prompt);

    QTextCharFormat cmdF=m_table->cellAt(0, 1).format();
    cmdF.setProperty(Cantor::DefaultHighlighter::BlockTypeProperty, Cantor::DefaultHighlighter::CommandBlock);
    m_table->cellAt(0, 1).setFormat(cmdF);

    //m_table->mergeCells(0, 1, 1, 2);
    m_commandCell=m_table->cellAt(0, 1);
}

CommandEntry::~CommandEntry()
{

}

int CommandEntry::type()
{
    return Type;
}

QString CommandEntry::command()
{
    QTextCursor c=m_commandCell.firstCursorPosition();
    c.setPosition(m_commandCell.lastCursorPosition().position(), QTextCursor::KeepAnchor);
    QString cmd=c.selectedText();
    cmd.replace(QChar::ParagraphSeparator, '\n'); //Replace the U+2029 paragraph break by a Normal Newline
    cmd.replace(QChar::LineSeparator, '\n'); //Replace the line break by a Normal Newline

    return cmd;
}

void CommandEntry::setExpression(Cantor::Expression* expr)
{
    if ( m_expression )
        m_expression->deleteLater();

    // Delete any previus error and/or result
    if(m_errorCell.isValid())
    {
        m_table->removeRows(m_errorCell.row(), 1);
        m_errorCell=QTextTableCell();
    }

    removeResult();

    foreach(const QTextTableCell& cell, m_informationCells)
    {
        m_table->removeRows(cell.row()-1, 2);
    }
    m_informationCells.clear();

    // Delete any previous result
    if (m_table && m_resultCell.isValid())
    {
        m_table->removeRows(m_resultCell.row(),  m_resultCell.rowSpan());
        m_resultCell=QTextTableCell();
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
        m_worksheet->gotResult(expr);
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

QTextCursor CommandEntry::firstValidCursorPosition()
{
    return m_commandCell.firstCursorPosition();
}

QTextCursor CommandEntry::lastValidCursorPosition()
{
    return m_commandCell.lastCursorPosition();
}

QTextCursor CommandEntry::closestValidCursor(const QTextCursor& cursor)
{
    if (firstValidCursorPosition().position() <= cursor.position() &&
            cursor.position() <= lastValidCursorPosition().position())
    {
        kDebug()<<"In CommandCell";
        return cursor;
    }
    else
        return firstValidCursorPosition();
}

bool CommandEntry::isValidCursor(const QTextCursor& cursor)
{
    return isInCommandCell(cursor)
            || isInCurrentInformationCell(cursor)
            || isInResultCell(cursor)
            || isInErrorCell(cursor);
}

bool CommandEntry::worksheetShortcutOverrideEvent(QKeyEvent* event, const QTextCursor& cursor)
{
    if (WorksheetEntry::worksheetShortcutOverrideEvent(event, cursor))
        return true;

    if (event->key() == Qt::Key_Tab && m_worksheet->completionEnabled())
    {
        // special tab handling here
        // get the current line of the entry. If it's empty, do a regular tab(indent),
        // otherwise check for tab completion (if supported by the backend)
        const QString line=currentLine(cursor).trimmed();
        if(line.isEmpty())
            return false;
        return true;
    }

    return false;
}

bool CommandEntry::worksheetKeyPressEvent(QKeyEvent* event, const QTextCursor& cursor)
{
    if (WorksheetEntry::worksheetKeyPressEvent(event, cursor))
    {
        return true;
    }
    else if (event->modifiers() == Qt::NoModifier
            && (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
            && isShowingCompletionPopup())
    {
        applySelectedCompletion();
        return true;
    }
    else if (!(isInCommandCell(cursor) || isInCurrentInformationCell(cursor)))
    {
        return true;
    }

    return false;
}

bool CommandEntry::worksheetMousePressEvent(QMouseEvent* event, const QTextCursor& cursor)
{
    Q_UNUSED(event);

    if (!isValidCursor(cursor))
        return true;

    return false;
}

bool CommandEntry::worksheetContextMenuEvent(QContextMenuEvent* event, const QTextCursor& cursor)
{
    if(isInResultCell(cursor) && expression() && expression()->result())
    {
        kDebug()<<"context menu in result...";
        KMenu* popup=new ResultContextMenu(this, m_worksheet);

        QMenu* defaultMenu=m_worksheet->mousePopupMenu();
        defaultMenu->setTitle(i18n("Other"));
        popup->addMenu(defaultMenu);

        popup->popup(event->globalPos());

        return true;
    }else if(isInCommandCell(cursor))
    {
        KMenu* defaultMenu = new KMenu(m_worksheet);

        defaultMenu->addAction(KStandardAction::cut(m_worksheet));
        defaultMenu->addAction(KStandardAction::copy(m_worksheet));
        defaultMenu->addAction(KStandardAction::paste(m_worksheet));
        defaultMenu->addSeparator();

        if(!m_worksheet->isRunning())
        {
            defaultMenu->addAction(KIcon("system-run"),i18n("Evaluate Worksheet"),m_worksheet,SLOT(evaluate()),0);
            if (!isEmpty())
                defaultMenu->addAction(i18n("Evaluate Entry"),m_worksheet,SLOT(evaluateCurrentEntry()),0);
        }
        else
            defaultMenu->addAction(KIcon("process-stop"),i18n("Interrupt"),m_worksheet,SLOT(interrupt()),0);
        defaultMenu->addSeparator();

        defaultMenu->addAction(KIcon("edit-delete"),i18n("Remove Entry"), m_worksheet, SLOT(removeCurrentEntry()));

	createSubMenuInsert(defaultMenu);

        defaultMenu->popup(event->globalPos());

        return true;
    }

    return false;
}

bool CommandEntry::acceptRichText()
{
    return false;
}

bool CommandEntry::acceptsDrop(const QTextCursor& cursor)
{
    return isInCommandCell(cursor);
}

void CommandEntry::setContent(const QString& content)
{
    firstValidCursorPosition().insertText(content);
}

void CommandEntry::setContent(const QDomElement& content, const KZip& file)
{
    firstValidCursorPosition().insertText(content.firstChildElement("Command").text());

    LoadedExpression* expr=new LoadedExpression( m_worksheet->session() );
    expr->loadFromXml(content, file);

    setExpression(expr);
}

void CommandEntry::showCompletion()
{
    //get the current line of the entry. If it's empty, ignore the call,
    //otherwise check for tab completion (if supported by the backend)
    const QString line=currentLine(m_worksheet->textCursor()); //.trimmed();

    if(line.trimmed().isEmpty())
    {
        return;
    }else
    {
        Cantor::CompletionObject* tco=m_worksheet->session()->completionFor(line, m_worksheet->textCursor().positionInBlock());
        if(tco)
            setCompletion(tco);
    }
}

QString CommandEntry::toPlain(QString& commandSep, QString& commentStartingSeq, QString& commentEndingSeq)
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

QString CommandEntry::currentLine(const QTextCursor& cursor)
{
    if(!isInCommandCell(cursor))
        return QString();

    QTextBlock block=m_worksheet->document()->findBlock(cursor.position());

    return block.text();
}

bool CommandEntry::evaluate(bool current)
{
    if (!current)
        return evaluateCommand();

    const QTextCursor c=m_worksheet->textCursor();
    if (isInCommandCell(c))
    {
        return evaluateCommand();
    }else if (isInCurrentInformationCell(c))
    {
        addInformation();
        return true;
    }
    return false;
}

bool CommandEntry::evaluateCommand()
{
    removeContextHelp();

    QString cmd = command();
    kDebug()<<"evaluating: "<<cmd;

    Cantor::Expression* expr;
    if(cmd.isEmpty())
        return false;

    expr=m_worksheet->session()->evaluateExpression(cmd);
    connect(expr, SIGNAL(gotResult()), m_worksheet, SLOT(gotResult()));

    setExpression(expr);

    return true;
}

void CommandEntry::interruptEvaluation()
{
    Cantor::Expression* expr=expression();
    if(expr)
        expr->interrupt();
}

void CommandEntry::update()
{
    if (m_expression==0||m_expression->result()==0)  //Don't crash if we don't have a result
        return;

    if (m_expression->result()->type()==Cantor::HelpResult::Type) return;  //Help is handled elsewhere

    if(!m_resultCell.isValid())
    {
        int row=0;
        if(actualInformationCell().isValid())
            row=actualInformationCell().row()+1;
        else
            row=m_commandCell.row()+1;
        m_table->insertRows(row, 1);
        //m_table->mergeCells(row, 1, 1, 2);
        m_resultCell=m_table->cellAt(row, 1);
        QTextCharFormat resF=m_table->cellAt(0, 1).format();
        resF.setProperty(Cantor::DefaultHighlighter::BlockTypeProperty, Cantor::DefaultHighlighter::ResultBlock);
        m_resultCell.setFormat(resF);
    }

    QTextBlockFormat block;
    block.setAlignment(Qt::AlignJustify);
    block.setProperty(Cantor::DefaultHighlighter::BlockTypeProperty, Cantor::DefaultHighlighter::ResultBlock);
    QTextCursor cursor(m_resultCell.firstCursorPosition());
    cursor.setBlockFormat(block);
    cursor.setPosition(m_resultCell.lastCursorPosition().position(), QTextCursor::KeepAnchor);

    kDebug()<<"setting cell to "<<m_expression->result()->toHtml();

    m_worksheet->resultProxy()->insertResult(cursor, m_expression->result());

    m_worksheet->ensureCursorVisible();
}

void CommandEntry::expressionChangedStatus(Cantor::Expression::Status status)
{
    QString text;
    switch (status)
    {
        case Cantor::Expression::Error:
            text=m_expression->errorMessage();
        break;
        case Cantor::Expression::Interrupted:
            text=i18n("Interrupted");
        break;
    }

    if(text.isEmpty())
        return;

    QTextCursor c;
    if(!m_errorCell.isValid())
    {
        int row;
        if(actualInformationCell().isValid())
            row=actualInformationCell().row()+1;
        else
            row=commandCell().row()+1;
        m_table->insertRows(row, 1);
        m_errorCell=m_table->cellAt(row, 1);
        QTextCharFormat errF=m_table->cellAt(0, 1).format();
        errF.setProperty(Cantor::DefaultHighlighter::BlockTypeProperty, Cantor::DefaultHighlighter::ErrorBlock);
        m_errorCell.setFormat(errF);

        c=m_errorCell.firstCursorPosition();
    }else
    {
        c=m_errorCell.firstCursorPosition();
        c.setPosition(m_errorCell.lastCursorPosition().position(),  QTextCursor::KeepAnchor);
    }

    c.insertHtml(text);
}

bool CommandEntry::isEmpty()
{
    QTextCursor c=m_commandCell.firstCursorPosition();
    c.setPosition(m_commandCell.lastCursorPosition().position(),  QTextCursor::KeepAnchor);
    QString text=c.selectedText();
    if(m_resultCell.isValid())
    {
        c=m_resultCell.firstCursorPosition();
        c.setPosition( m_resultCell.lastCursorPosition().position(),  QTextCursor::KeepAnchor);
        text+=c.selectedText();
    }
    text.remove(QRegExp("[\n\t\r]"));
    kDebug()<<"text: "<<text;
    return text.trimmed().isEmpty();
}

void CommandEntry::setCompletion(Cantor::CompletionObject* tc)
{
    if(m_completionObject)
        m_completionObject->deleteLater();

    m_completionObject=tc;
    connect(tc, SIGNAL(done()), this, SLOT(showCompletions()));
}

void CommandEntry::showCompletions()
{
    QString completion=m_completionObject->makeCompletion(m_completionObject->command());
    kDebug()<<"completion: "<<completion;
    kDebug()<<"showing "<<m_completionObject->allMatches();

    completeCommandTo(completion);

    if(m_completionObject->hasMultipleMatches())
    {
        QToolTip::showText(QPoint(), QString(), m_worksheet);
        switch(Settings::self()->completionStyle())
        {
            case Settings::PopupCompletion:
            {
                m_completionBox=new KCompletionBox(m_worksheet);
                m_completionBox->setItems(m_completionObject->allMatches());
                m_completionBox->setTabHandling(true);
                m_completionBox->setActivateOnSelect(true);
                connect(m_completionBox, SIGNAL(activated(const QString&)), this, SLOT(completeCommandTo(const QString&)));
                connect(m_worksheet, SIGNAL(textChanged()), m_completionBox, SLOT(deleteLater()));

                QRect rect=m_worksheet->cursorRect();
                kDebug()<<"cursor is within: "<<rect;
                const QPoint popupPoint=rect.bottomLeft();
                m_completionBox->popup();
                m_completionBox->move(m_worksheet->mapToGlobal(popupPoint));
                break;
            }
            case Settings::InlineCompletion:
            {
                int oldCursorPos=m_worksheet->textCursor().position();

                //Show a list of possible completions
                if(!m_contextHelpCell.isValid())
                {
                    //remember the actual cursor position, and reset the cursor later
                    int row=m_commandCell.row()+1;

                    m_table->insertRows(row, 1);
                    m_contextHelpCell=m_table->cellAt(row, 1);

                    QTextCursor c=m_worksheet->textCursor();
                    c.setPosition(oldCursorPos);
                    m_worksheet->setTextCursor(c);
                }

                QTextCursor cursor=m_contextHelpCell.firstCursorPosition();
                cursor.setPosition(m_contextHelpCell.lastCursorPosition().position(), QTextCursor::KeepAnchor);

                int count=0;
                QString html="<table>";
                const QStringList& matches=m_completionObject->allMatches();
                foreach(const QString& item, matches)
                {
                    html+="<tr><td>"+item+"</td></tr>";
                    count++;
                    if(count>10)
                        break;
                }

                const int itemsLeft=matches.size()-count;
                if(itemsLeft>0)
                    html+="<tr><td><b>"+i18n("And %1 more...", itemsLeft)+"<b></td></tr>";

                html+="</table>";

                cursor.insertHtml(html);

                m_worksheet->setTextCursor(cursor);
                m_worksheet->ensureCursorVisible();
                QTextCursor oldC=m_worksheet->textCursor();
                oldC.setPosition(oldCursorPos);
                m_worksheet->setTextCursor(oldC);
                m_worksheet->ensureCursorVisible();
                break;
            }
        }

    }else
    {
        //remove the list if it isn't needed anymore
        removeContextHelp();

        QString cmd=currentLine(m_worksheet->textCursor());
        if(cmd.endsWith('('))
            cmd.chop(1);

        int brIndex=cmd.lastIndexOf('(')+1;
        int semIndex=cmd.lastIndexOf(';')+1;
        int spaceIndex=cmd.lastIndexOf(' ')+1;

        cmd=cmd.mid(qMax(brIndex, qMax(semIndex, spaceIndex)));

        Cantor::SyntaxHelpObject* obj=m_worksheet->session()->syntaxHelpFor(cmd);
        if(obj)
            setSyntaxHelp(obj);
    }


}

bool CommandEntry::isShowingCompletionPopup()
{

    return m_completionBox&&m_completionBox->isVisible();
}

void CommandEntry::applySelectedCompletion()
{
    QListWidgetItem* item=m_completionBox->currentItem();
    if(item)
        completeCommandTo(item->text());
    m_completionBox->hide();
}

void CommandEntry::completeCommandTo(const QString& completion)
{
    //replace the current command with the completion
    QTextCursor cursor=m_worksheet->textCursor();
    if(!isInCommandCell(cursor)) return;

    QTextCursor beginC=m_worksheet->document()->find(m_completionObject->command(), cursor, QTextDocument::FindBackward);
    beginC.setPosition(cursor.position(), QTextCursor::KeepAnchor);
    beginC.insertHtml(completion);
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
    const QString& msg=m_syntaxHelpObject->toHtml();
    const QRect r=m_worksheet->cursorRect();
    const QPoint pos=m_worksheet->mapToGlobal(r.topLeft());

    QTextCursor entryCursor=m_table->firstCursorPosition();
    entryCursor.setPosition(m_table->lastCursorPosition().position(), QTextCursor::KeepAnchor);
    QRect tableRect=m_worksheet->cursorRect(entryCursor);

    QToolTip::showText(pos, msg, m_worksheet);
}

void CommandEntry::resultDeleted()
{
    kDebug()<<"result got removed...";
}

QTextTable* CommandEntry::table()
{
    return m_table;
}

QTextTableCell CommandEntry::commandCell()
{
    return m_commandCell;
}

QTextTableCell CommandEntry::actualInformationCell()
{
    if(m_informationCells.isEmpty())
        return QTextTableCell();
    else
        return m_informationCells.last();
}

QTextTableCell CommandEntry::resultCell()
{
    return m_resultCell;
}

void CommandEntry::addInformation()
{
    QTextCursor c=actualInformationCell().firstCursorPosition();
    c.setPosition(actualInformationCell().lastCursorPosition().position(), QTextCursor::KeepAnchor);
    QString inf=c.selectedText();

    inf.replace(QChar::ParagraphSeparator, '\n'); //Replace the U+2029 paragraph break by a Normal Newline
    inf.replace(QChar::LineSeparator, '\n'); //Replace the line break by a Normal Newline

    kDebug()<<"adding information: "<<inf;
    if(m_expression)
        m_expression->addInformation(inf);
}

void CommandEntry::showAdditionalInformationPrompt(const QString& question)
{
    int row;
    if (actualInformationCell().isValid())
        row=actualInformationCell().row()+1;
    else
        row=commandCell().row()+1;

    //insert two rows, one for the question, one for the answer
    m_table->insertRows(row, 2);

    QTextTableCell cell=m_table->cellAt(row, 1);
    cell.firstCursorPosition().insertText(question);
    cell=m_table->cellAt(row+1, 1);
    m_informationCells.append(cell);

    m_worksheet->setTextCursor(cell.firstCursorPosition());
    m_worksheet->ensureCursorVisible();
    m_worksheet->setCurrentEntry(this, false);
}

bool CommandEntry::isInCurrentInformationCell(const QTextCursor& cursor)
{
    if(m_informationCells.isEmpty())
        return false;

    QTextTableCell cell=m_informationCells.last();
    if(cursor.position()>=cell.firstCursorPosition().position()&&cursor.position()<=cell.lastCursorPosition().position())
        return true;
    else
        return false;
}

bool CommandEntry::isInCommandCell(const QTextCursor& cursor)
{
    if(cursor.position()>=m_commandCell.firstCursorPosition().position()&&cursor.position()<=m_commandCell.lastCursorPosition().position())
        return true;
    else
        return false;
}

bool CommandEntry::isInPromptCell(const QTextCursor& cursor)
{
    const QTextTableCell cell=m_table->cellAt(0, 0);
    if(cursor.position()>=cell.firstCursorPosition().position()&&cursor.position()<=cell.lastCursorPosition().position())
        return true;
    else
        return false;
}

bool CommandEntry::isInResultCell(const QTextCursor& cursor)
{
    if(!m_resultCell.isValid())
        return false;

    if(cursor.position()>=m_resultCell.firstCursorPosition().position()&&cursor.position()<=m_resultCell.lastCursorPosition().position())
        return true;
    else
        return false;
}

bool CommandEntry::isInErrorCell(const QTextCursor& cursor)
{
    if(!m_errorCell.isValid())
        return false;

    if(cursor.position()>=m_errorCell.firstCursorPosition().position()&&cursor.position()<=m_errorCell.lastCursorPosition().position())
        return true;
    else
        return false;
}

void CommandEntry::checkForSanity()
{
    QTextTableCell cell=m_table->cellAt(0, 0);
    QTextCursor c=cell.firstCursorPosition();
    c.setPosition(cell.lastCursorPosition().position(), QTextCursor::KeepAnchor);
    if(c.selectedText()!=CommandEntry::Prompt)
        updatePrompt();
}

void CommandEntry::removeResult()
{
    if(m_resultCell.isValid())
    {
        m_table->removeRows(m_resultCell.row(), 1);
        m_resultCell=QTextTableCell();
    }

    if(m_expression)
    {
        m_expression->clearResult();
    }

}


void CommandEntry::removeContextHelp()
{
    if(m_completionObject)
        m_completionObject->deleteLater();

    m_completionObject=0;
    if(m_contextHelpCell.isValid())
    {
        m_table->removeRows(m_contextHelpCell.row(), 1);
        m_contextHelpCell=QTextTableCell();
    }
}

void CommandEntry::updatePrompt()
{
    KColorScheme color = KColorScheme( QPalette::Normal, KColorScheme::View);
    QTextTableCell cell=m_table->cellAt(0, 0);
    QTextCursor c=cell.firstCursorPosition();
    QTextCharFormat cformat = c.charFormat();

    cformat.clearForeground();
    c.setPosition(cell.lastCursorPosition().position(), QTextCursor::KeepAnchor);
    c.setCharFormat(cformat);
    cformat.setFontWeight(QFont::Bold);

    //insert the session id if available
    if(m_expression&&m_worksheet->showExpressionIds())
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

void CommandEntry::invalidate()
{
    m_table=0;
    m_commandCell=QTextTableCell();
    m_contextHelpCell=QTextTableCell();
    m_informationCells.clear();
    m_errorCell=QTextTableCell();
    m_resultCell=QTextTableCell();
}
