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

WorksheetEntry::WorksheetEntry( QTextCursor cursor,Worksheet* parent ) : QObject( parent  )
{
    QTextFrameFormat format;
    //KColorScheme scheme(QPalette::Active,  KColorScheme::Window);
    //format.setBackground( scheme.background(KColorScheme::AlternateBackground) );
    format.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    format.setBorder(1.0);

    m_cmdFrame=cursor.insertFrame(format);
    m_cmdFrame->firstCursorPosition().insertText(Prompt);

    m_resultFrame=0;
    m_expression=0;
    m_worksheet=parent;
    kDebug()<<startPosition()<<"\t"<<resultStartPosition()<<"\t"<<endPosition();
}

WorksheetEntry::~WorksheetEntry()
{

}

QString WorksheetEntry::command()
{
    kDebug()<<"command for entry from ";
    kDebug()<<startPosition()<<"\t"<<resultStartPosition()<<"\t"<<endPosition();

    QTextCursor cursor=cmdCursor();
    cursor.setPosition(resultStartPosition()-1, QTextCursor::KeepAnchor);
    QString cmd=cursor.selectedText().trimmed();
    cmd.replace(8233, '\n'); //Replace the U+2029 paragraph break by a Normal Newline

    return cmd;

}

void WorksheetEntry::setExpression(MathematiK::Expression* expr)
{
    if ( m_expression )
        m_expression->deleteLater();
    m_expression=expr;
    connect(expr, SIGNAL(gotResult()), this, SLOT(updateResult()));
}

MathematiK::Expression* WorksheetEntry::expression()
{
    return m_expression;
}

void WorksheetEntry::updateResult()
{
    if (m_expression->result()->type()==MathematiK::HelpResult::Type) return;  //Help is handled elsewhere

    if(m_resultFrame==0)
    {
        QTextFrameFormat format;
        format.setLeftMargin(100);
        format.setRightMargin(100);
        QTextCursor c(m_cmdFrame->lastCursorPosition());
        m_resultFrame=c.insertFrame(format);
    }

    QTextBlockFormat block;
    block.setAlignment(Qt::AlignJustify);
    QTextCursor cursor(resultCursor());
    cursor.setBlockFormat(block);
    cursor.setPosition(endPosition(), QTextCursor::KeepAnchor);
    kDebug()<<"setting cell to "<<m_expression->result()->toHtml();
    if(m_expression->result()->toHtml().trimmed().isEmpty())
        cursor.removeSelectedText();
    else
        cursor.insertHtml(m_expression->result()->toHtml());

    m_worksheet->ensureCursorVisible();
}

int WorksheetEntry::startPosition()
{
    return m_cmdFrame->firstPosition()+Prompt.size();
}

int WorksheetEntry::resultStartPosition()
{
    if(m_resultFrame)
        return m_resultFrame->firstPosition();
    else
        return (m_cmdFrame->lastPosition()+1);
}

int WorksheetEntry::endPosition()
{
    if(m_resultFrame)
        return m_resultFrame->lastPosition();
    else
        return (m_cmdFrame->lastPosition()+1);
}

QTextCursor WorksheetEntry::cmdCursor()
{
    QTextCursor c=m_cmdFrame->firstCursorPosition();
    c.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, Prompt.length());
    return c;
}

QTextCursor WorksheetEntry::resultCursor()
{
    return m_resultFrame->firstCursorPosition();
}

QTextCursor WorksheetEntry::endCursor()
{
    return m_resultFrame->lastCursorPosition();
}

bool WorksheetEntry::isEmpty()
{
    QTextCursor c=cmdCursor();
    c.setPosition(resultStartPosition()-1, QTextCursor::KeepAnchor);
    QString text=c.selectedText();
    c.setPosition( resultStartPosition());
    c.setPosition( endPosition(), QTextCursor::KeepAnchor);
    text+=c.selectedText();
    text.remove(QRegExp("[\n\t\r]"));
    kDebug()<<"text: "<<text;
    return text.trimmed().isEmpty();
}

void WorksheetEntry::setResult(const QString& html)
{
    if(m_resultFrame==0)
    {
        QTextFrameFormat format;
        format.setLeftMargin(100);
        format.setRightMargin(100);
        QTextCursor c(m_cmdFrame->lastCursorPosition());
        m_resultFrame=c.insertFrame(format);
    }


    QTextBlockFormat block;
    block.setAlignment(Qt::AlignJustify);
    QTextCursor cursor(resultCursor());
    cursor.setBlockFormat(block);
    cursor.setPosition(endPosition(), QTextCursor::KeepAnchor);
    cursor.insertHtml(html);

    cursor.setPosition(resultStartPosition()-1);
    m_worksheet->setTextCursor(cursor);
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

#include "worksheetentry.moc"
