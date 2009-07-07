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

#include "worksheetentry.h"

#include "lib/expression.h"
#include "lib/result.h"
#include "lib/helpresult.h"
#include "lib/tabcompletionobject.h"
#include "worksheet.h"

#include <QTextDocument>
#include <QTextFrame>
#include <QToolTip>
#include <kdebug.h>
#include <kglobal.h>
#include <kcolorscheme.h>
#include <kcompletionbox.h>

const QString WorksheetEntry::Prompt=">>> ";

WorksheetEntry::WorksheetEntry( QTextCursor position,Worksheet* parent ) : QObject( parent  )
{
    m_expression=0;
    m_worksheet=parent;
    m_tabCompletionObject=0;

    QTextTableFormat tableFormat;
    QVector<QTextLength> constraints;
    QFontMetrics metrics(parent->document()->defaultFont());
    constraints<< QTextLength(QTextLength::FixedLength, metrics.width(WorksheetEntry::Prompt))
               <<QTextLength(QTextLength::PercentageLength, 100);

    tableFormat.setColumnWidthConstraints(constraints);
    tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    tableFormat.setCellSpacing(10);
    tableFormat.setTopMargin(5);

    QTextFrameFormat frameFormat;
    frameFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    frameFormat.setBorder(1);

    position=(position.insertFrame(frameFormat))->firstCursorPosition();

    m_table=position.insertTable(1, 2, tableFormat);
    connect(m_table, SIGNAL(destroyed(QObject*)), this, SLOT(deleteLater()));

    m_table->cellAt(0, 0).firstCursorPosition().insertText(Prompt);
    //m_table->mergeCells(0, 1, 1, 2);
    m_commandCell=m_table->cellAt(0, 1);
}

WorksheetEntry::~WorksheetEntry()
{

}

QString WorksheetEntry::command()
{
    QTextCursor c=m_commandCell.firstCursorPosition();
    c.setPosition(m_commandCell.lastCursorPosition().position(), QTextCursor::KeepAnchor);
    QString cmd=c.selectedText();
    cmd.replace(QChar::ParagraphSeparator, '\n'); //Replace the U+2029 paragraph break by a Normal Newline

    return cmd;
}

void WorksheetEntry::setExpression(MathematiK::Expression* expr)
{
    if ( m_expression )
        m_expression->deleteLater();
    m_expression=expr;

    if(m_errorCell.isValid())
    {
        m_table->removeRows(m_errorCell.row(), 1);
        m_errorCell=QTextTableCell();
    }
    foreach(QTextTableCell cell, m_informationCells)
    {
        m_table->removeRows(cell.row(), 2);
    }
    m_informationCells.clear();

    connect(expr, SIGNAL(gotResult()), this, SLOT(updateResult()));
    connect(expr, SIGNAL(statusChanged(MathematiK::Expression::Status)), this, SLOT(expressionChangedStatus(MathematiK::Expression::Status)));
    connect(expr, SIGNAL(needsAdditionalInformation(const QString&)), this, SLOT(showAdditionalInformationPrompt(const QString&)));
}

MathematiK::Expression* WorksheetEntry::expression()
{
    return m_expression;
}

QString WorksheetEntry::currentLine(const QTextCursor& cursor)
{
    if(!isInCommandCell(cursor))
        return QString();

    QTextBlock block=m_worksheet->document()->findBlock(cursor.position());

    return block.text();
}

void WorksheetEntry::updateResult()
{
    if (m_expression->result()->type()==MathematiK::HelpResult::Type) return;  //Help is handled elsewhere

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
    }

    QTextBlockFormat block;
    block.setAlignment(Qt::AlignJustify);
    QTextCursor cursor(m_resultCell.firstCursorPosition());
    cursor.setBlockFormat(block);
    cursor.setPosition(m_resultCell.lastCursorPosition().position(), QTextCursor::KeepAnchor);

    QVariant data=m_expression->result()->data();
    if(data.type() == QVariant::Image)
    {
        //It's an image. add it as a ressource to the worksheet, so it can be referenced by an url
        static int resourceIdCounter=0;
        QUrl resourceUrl=KUrl(QString("mydata://image_%1").arg(resourceIdCounter++));

        m_worksheet->document()->addResource(QTextDocument::ImageResource, resourceUrl,  data.value<QImage>());
        m_expression->result()->setResourceUrl(resourceUrl);
    }

    kDebug()<<"setting cell to "<<m_expression->result()->toHtml();
    if(m_expression->result()->toHtml().trimmed().isEmpty())
        cursor.removeSelectedText();
    else
        cursor.insertHtml(m_expression->result()->toHtml());

    m_worksheet->ensureCursorVisible();
}

void WorksheetEntry::expressionChangedStatus(MathematiK::Expression::Status status)
{
    if(status==MathematiK::Expression::Error)
    {
        if(!m_errorCell.isValid())
        {
            int row;
            if(actualInformationCell().isValid())
                row=actualInformationCell().row()+1;
            else
                row=commandCell().row()+1;
            m_table->insertRows(row, 1);
            m_errorCell=m_table->cellAt(row, 1);
        }

        m_errorCell.firstCursorPosition().insertText(m_expression->errorMessage());
    }
}

bool WorksheetEntry::isEmpty()
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

void WorksheetEntry::setResult(const QString& html)
{
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
    }

    removeContextHelp();

    QTextBlockFormat block;
    block.setAlignment(Qt::AlignJustify);
    QTextCursor cursor(m_resultCell.firstCursorPosition());
    cursor.setBlockFormat(block);
    cursor.setPosition(m_resultCell.lastCursorPosition().position(), QTextCursor::KeepAnchor);

    if(html.isEmpty())
        cursor.removeSelectedText();
    else
        cursor.insertHtml(html);

    kDebug()<<"setting to "<<html;
    m_worksheet->ensureCursorVisible();
}

void WorksheetEntry::setTabCompletion(MathematiK::TabCompletionObject* tc)
{
    if(m_tabCompletionObject)
        m_tabCompletionObject->deleteLater();

    m_tabCompletionObject=tc;
    connect(tc, SIGNAL(done()), this, SLOT(applyTabCompletion()));
}

void WorksheetEntry::applyTabCompletion()
{
    kDebug()<<"showing "<<m_tabCompletionObject->completions();
    m_tabCompletionObject->makeCompletion(m_tabCompletionObject->command());
    if(m_tabCompletionObject->hasMultipleMatches())
    {
        //Show a list of possible completions
        if(!m_contextHelpCell.isValid())
        {
            //remember the actual cursor position, and reset the cursor later
            int cursorPos=m_worksheet->textCursor().position();
            int row=m_commandCell.row()+1;

            m_table->insertRows(row, 1);
            m_contextHelpCell=m_table->cellAt(row, 1);

            QTextCursor c=m_worksheet->textCursor();
            c.setPosition(cursorPos);
            m_worksheet->setTextCursor(c);
        }

        QTextCursor cursor=m_contextHelpCell.firstCursorPosition();
        cursor.setPosition(m_contextHelpCell.lastCursorPosition().position(),  QTextCursor::KeepAnchor);

        QString html="<table>";
        foreach(const QString& item, m_tabCompletionObject->allMatches())
        {
            html+="<tr><td>"+item+"</td></tr>";
        }
        html+="</table>";

        cursor.insertHtml(html);

    }else
    {
        //there is only one possible completion, so replace the
        //current command with the match
        QTextCursor cursor=m_worksheet->textCursor();
        if(!isInCommandCell(cursor)) return;

        QTextCursor beginC=m_worksheet->document()->find(m_tabCompletionObject->command(), cursor, QTextDocument::FindBackward);
        beginC.setPosition(cursor.position(), QTextCursor::KeepAnchor);
        beginC.insertHtml(m_tabCompletionObject->makeCompletion(m_tabCompletionObject->command()));

        removeContextHelp();
    }

}

void WorksheetEntry::resultDeleted()
{
    kDebug()<<"result got removed...";
}

QTextTableCell WorksheetEntry::commandCell()
{
    return m_commandCell;
}

QTextTableCell WorksheetEntry::actualInformationCell()
{
    if(m_informationCells.isEmpty())
        return QTextTableCell();
    else
        return m_informationCells.last();
}

QTextTableCell WorksheetEntry::resultCell()
{
    return m_resultCell;
}

void WorksheetEntry::addInformation()
{
    QTextCursor c=actualInformationCell().firstCursorPosition();
    c.setPosition(actualInformationCell().lastCursorPosition().position(), QTextCursor::KeepAnchor);
    QString inf=c.selectedText();
    if(!inf.endsWith(";"))
        inf+=";";
    inf.replace(QChar::ParagraphSeparator, '\n'); //Replace the U+2029 paragraph break by a Normal Newline
    kDebug()<<"adding information: "<<inf;
    if(m_expression)
        m_expression->addInformation(inf);
}

void WorksheetEntry::showAdditionalInformationPrompt(const QString& question)
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
}

bool WorksheetEntry::contains(const QTextCursor& cursor)
{
    if(cursor.position()>=m_table->firstCursorPosition().position()&&cursor.position()<=m_table->lastCursorPosition().position())
        return true;
    else
        return false;
}

int WorksheetEntry::firstPosition()
{
    return m_table->firstCursorPosition().position();
}

int WorksheetEntry::lastPosition()
{
    return m_table->lastCursorPosition().position();
}

bool WorksheetEntry::isInCurrentInformationCell(const QTextCursor& cursor)
{
    QTextTableCell cell=m_informationCells.last();
    if(cursor.position()>=cell.firstCursorPosition().position()&&cursor.position()<=cell.lastCursorPosition().position())
        return true;
    else
        return false;
}

bool WorksheetEntry::isInCommandCell(const QTextCursor& cursor)
{
    if(cursor.position()>=m_commandCell.firstCursorPosition().position()&&cursor.position()<=m_commandCell.lastCursorPosition().position())
        return true;
    else
        return false;
}

bool WorksheetEntry::isInPromptCell(const QTextCursor& cursor)
{
    const QTextTableCell cell=m_table->cellAt(0, 0);
    if(cursor.position()>=cell.firstCursorPosition().position()&&cursor.position()<=cell.lastCursorPosition().position())
        return true;
    else
        return false;
}

void WorksheetEntry::checkForSanity()
{
    QTextTableCell cell=m_table->cellAt(0, 0);
    QTextCursor c=cell.firstCursorPosition();
    c.setPosition(cell.lastCursorPosition().position(), QTextCursor::KeepAnchor);
    if(c.selectedText()!=WorksheetEntry::Prompt)
        c.insertText(WorksheetEntry::Prompt);
}

void WorksheetEntry::removeContextHelp()
{
    if(m_tabCompletionObject)
        m_tabCompletionObject->deleteLater();

    m_tabCompletionObject=0;
    if(m_contextHelpCell.isValid())
    {
        m_table->removeRows(m_contextHelpCell.row(), 1);
        m_contextHelpCell=QTextTableCell();
    }
}

#include "worksheetentry.moc"
