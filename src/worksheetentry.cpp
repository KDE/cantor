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
#include "worksheet.h"

#include <QTextDocument>
#include <QTextFrame>
#include <QToolTip>
#include <kdebug.h>
#include <kglobal.h>
#include <kcolorscheme.h>

const QString WorksheetEntry::Prompt=">>> ";

WorksheetEntry::WorksheetEntry( int position,Worksheet* parent ) : QObject( parent  )
{
    m_expression=0;
    m_worksheet=parent;

    m_worksheet->mainTable()->insertRows(position, 1);
    m_worksheet->mainTable()->cellAt(position, 0).firstCursorPosition().insertText(Prompt);
    m_worksheet->mainTable()->mergeCells(position, 1, 1, 2);
    m_commandCell=m_worksheet->mainTable()->cellAt(position, 1);
}

WorksheetEntry::~WorksheetEntry()
{

}

QString WorksheetEntry::command()
{
    QTextCursor c=m_commandCell.firstCursorPosition();
    c.setPosition(m_commandCell.lastCursorPosition().position(), QTextCursor::KeepAnchor);
    QString cmd=c.selectedText();
    cmd.replace(8233, '\n'); //Replace the U+2029 paragraph break by a Normal Newline

    return cmd;
}

void WorksheetEntry::setExpression(MathematiK::Expression* expr)
{
    if ( m_expression )
        m_expression->deleteLater();
    m_expression=expr;

    foreach(QTextTableCell cell, m_informationCells)
    {
        m_worksheet->mainTable()->removeRows(cell.row(), 1);
    }
    m_informationCells.clear();

    connect(expr, SIGNAL(gotResult()), this, SLOT(updateResult()));
    connect(expr, SIGNAL(needsAdditionalInformation(const QString&)), this, SLOT(showAdditionalInformationPrompt(const QString&)));
}

MathematiK::Expression* WorksheetEntry::expression()
{
    return m_expression;
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
        m_worksheet->mainTable()->insertRows(row, 1);
        m_worksheet->mainTable()->mergeCells(row, 1, 1, 2);
        m_resultCell=m_worksheet->mainTable()->cellAt(row, 1);
    }

    QTextBlockFormat block;
    block.setAlignment(Qt::AlignJustify);
    QTextCursor cursor(m_resultCell.firstCursorPosition());
    cursor.setBlockFormat(block);
    cursor.setPosition(m_resultCell.lastCursorPosition().position(), QTextCursor::KeepAnchor);
    kDebug()<<"setting cell to "<<m_expression->result()->toHtml();
    if(m_expression->result()->toHtml().trimmed().isEmpty())
        cursor.removeSelectedText();
    else
        cursor.insertHtml(m_expression->result()->toHtml());

    m_worksheet->ensureCursorVisible();
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
        m_worksheet->mainTable()->insertRows(row, 1);
        m_worksheet->mainTable()->mergeCells(row, 1, 1, 2);
        m_resultCell=m_worksheet->mainTable()->cellAt(row, 1);
    }

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

void WorksheetEntry::setContextHelp(MathematiK::Expression* expression)
{
    m_contextHelpExpression=expression;
    connect(expression, SIGNAL(gotResult()), this, SLOT(showContextHelp()));
}

void WorksheetEntry::showContextHelp()
{
    kDebug()<<"showing "<<m_contextHelpExpression->result()->toHtml();
    setResult(m_contextHelpExpression->result()->toHtml());
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
    inf.replace(8233, '\n'); //Replace the U+2029 paragraph break by a Normal Newline
    kDebug()<<"adding information: "<<inf;
    if(m_expression)
        m_expression->addInformation(inf);
}

void WorksheetEntry::showAdditionalInformationPrompt(const QString& question)
{
    m_worksheet->mainTable()->insertRows(m_commandCell.row()+1, 1);
    //Split the resulting cell in two parts. one for the question, one for the answer
    QTextTableCell cell=m_worksheet->mainTable()->cellAt(m_commandCell.row()+1, 1);
    cell.firstCursorPosition().insertText(question);
    cell=m_worksheet->mainTable()->cellAt(m_commandCell.row()+1, 2);
    m_informationCells.append(cell);

    m_worksheet->setTextCursor(cell.firstCursorPosition());
    m_worksheet->ensureCursorVisible();
}

#include "worksheetentry.moc"
