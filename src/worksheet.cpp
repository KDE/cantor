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

#include "worksheet.h"

#include "lib/backend.h"
#include "lib/session.h"
#include "lib/expression.h"
#include "lib/result.h"
#include "lib/helpresult.h"

#include "worksheetentry.h"

#include <QEvent>
#include <QKeyEvent>
#include <QTextCursor>
#include <QTextFrame>
#include <QTimer>
#include <QTextTable>
#include <QTextLength>
#include <QTextTableFormat>
#include <QFontMetrics>

#include <kdebug.h>
#include <kzip.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kurl.h>

Worksheet::Worksheet(MathematiK::Backend* backend, QWidget* parent) : QTextEdit(parent)
{
    m_session=backend->createSession();
    m_session->login();

    initMainTable();
    appendEntry();
}

Worksheet::~Worksheet()
{
    m_session->logout();
}

void Worksheet::initMainTable()
{
    QTextTableFormat tableFormat;
    QVector<QTextLength> constraints;
    QFontMetrics metrics(document()->defaultFont());
    constraints<< QTextLength(QTextLength::FixedLength, metrics.width(WorksheetEntry::Prompt))
               <<QTextLength(QTextLength::PercentageLength, 25)
               <<QTextLength(QTextLength::PercentageLength, 75);

    tableFormat.setColumnWidthConstraints(constraints);
    tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    tableFormat.setCellSpacing(5);

    m_mainTable=textCursor().insertTable(1, 3, tableFormat);
    m_mainTable->mergeCells(0, 0, 1, 3);
}

bool Worksheet::event(QEvent* event)
{
    if ( event->type() == QEvent::KeyPress )
    {
        QKeyEvent *ke = static_cast<QKeyEvent *>( event );
        if ( ke->key() == Qt::Key_Tab )
        {
            // special tab handling here
            WorksheetEntry* current=currentEntry();
            if (current)
            {
                current->setContextHelp(m_session->contextHelp(current->command()));
                return true;
            }

        }else if ( ( ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return ) && ke->modifiers() & Qt::ShiftModifier)
        {
            evaluateCurrentEntry();
            return true;
        }else if ( ke->key() == Qt::Key_Left )
        {
            bool r=QTextEdit::event(event);
            QTextTableCell cell=m_mainTable->cellAt(textCursor());
            if(cell.column()==0) //The cursor is placed at the prompt column. move it up if possible
            {
                 if(cell.row()>1)
                 {
                     WorksheetEntry* e;
                     //search for the next Entry in the cells above
                     for (int i=cell.row()-1;i>=0;i--)
                     {
                         e=entryAt(i);
                         if(e) break;
                     }
                     setTextCursor(e->commandCell().firstCursorPosition());
                 }
                else
                    setTextCursor(m_mainTable->cellAt(1, 1).firstCursorPosition());
            }
            return r;
        }else if ( ke->key() == Qt::Key_Right )
        {
            QTextTableCell cell=m_mainTable->cellAt(textCursor());
            WorksheetEntry* e=currentEntry();
            //search for the Entry this cell belongs to
            if(e&&textCursor().position()==e->commandCell().lastCursorPosition().position())
            {
                //search for the next Entry in the cells above
                for (int i=cell.row()+1;i<m_mainTable->rows();i++)
                {
                    e=entryAt(i);
                    if(e) break;
                }
                if(e)
                    setTextCursor(e->commandCell().firstCursorPosition());
                return true;
            }else
            {
                    bool r=QTextEdit::event(event);
                    cell=m_mainTable->cellAt(textCursor());
                    if(cell.column()==0) //The cursor is placed at the prompt column. move it right
                    {
                        setTextCursor(m_mainTable->cellAt(cell.row(), 1).firstCursorPosition());
                    }
                    return r;
            }
        }

        bool r=QTextEdit::event(event);
        foreach(WorksheetEntry* e, m_entries)
        {
            QTextTableCell cell=m_mainTable->cellAt(e->commandCell().row(), 0);
            QTextCursor c=cell.firstCursorPosition();
            c.setPosition(cell.lastCursorPosition().position(), QTextCursor::KeepAnchor);
            if(c.selectedText()!=WorksheetEntry::Prompt)
                c.insertText(WorksheetEntry::Prompt);
        }
        return r;
    }

    return QTextEdit::event(event);
}

void Worksheet::evaluate()
{

    emit modified();
}

void Worksheet::evaluateCurrentEntry()
{
    kDebug()<<"evaluation requested...";
    WorksheetEntry* entry=currentEntry();
    if (entry)
    {
        QString cmd=entry->command();
        MathematiK::Expression* expr;
        if(cmd.isEmpty()) return;

        expr=m_session->evaluateExpression(cmd);
        connect(expr, SIGNAL(gotResult()), this, SLOT(gotResult()));

        entry->setExpression(expr);

        if(!m_entries.last()->isEmpty())
            appendEntry();

        emit modified();
    }else
    {
        QTextTableCell cell=m_mainTable->cellAt(textCursor());
        foreach(WorksheetEntry* e, m_entries)
        {
            if (e->actualInformationCell()==cell)
            {
                e->addInformation();
                setTextCursor(m_mainTable->cellAt(cell.row()+1, 1).firstCursorPosition());
            }
        }
    }
}

WorksheetEntry* Worksheet::currentEntry()
{
    QTextTableCell cell=m_mainTable->cellAt(textCursor());
    return entryAt(cell.row());
}

WorksheetEntry* Worksheet::entryAt(int row)
{
   QTextTableCell cell=m_mainTable->cellAt(row, 1);
   if (cell.isValid())
   {
       foreach(WorksheetEntry* entry, m_entries)
       {
           if (entry->commandCell()==cell)
               return entry;
       }
   }
   return 0;
}

void Worksheet::appendEntry(const QString& text)
{
    WorksheetEntry* entry=new WorksheetEntry( m_mainTable->rows(),this);

    connect(entry, SIGNAL(destroyed(QObject*)), this, SLOT(removeEntry(QObject*)));
    m_entries.append(entry);

    setTextCursor(entry->commandCell().firstCursorPosition());
    ensureCursorVisible();

    if(!text.isNull())
    {
        //entry->cmdCursor().insertText(text);
        //evaluateCurrentEntry();
    }

}

void Worksheet::interrupt()
{
    m_session->interrupt();
}

void Worksheet::interruptCurrentExpression()
{
    MathematiK::Expression* expr=currentEntry()->expression();
    if(expr)
        expr->interrupt();
}

MathematiK::Session* Worksheet::session()
{
    return m_session;
}

bool Worksheet::isRunning()
{
    return m_session->status()==MathematiK::Session::Running;
}

QTextTable* Worksheet::mainTable()
{
    return m_mainTable;
}

void Worksheet::save( const QString& filename )
{
    kDebug()<<"saving to filename";
    KZip zipFile( filename );


    if ( !zipFile.open(QIODevice::WriteOnly) )
    {
        KMessageBox::error( this,  i18n( "Cannot write file %1:\n." , filename ),
                            i18n( "MathematiK" ));
        return;
    }

    QDomDocument doc( "MathematiKWorksheet" );
    QDomElement root=doc.createElement( "Worksheet" );
    root.setAttribute("backend", m_session->backend()->name());
    doc.appendChild(root);

    foreach( WorksheetEntry* entry, m_entries )
    {
        if ( entry->expression() )
            root.appendChild( entry->expression()->toXml(doc) );
    }

    QByteArray content=doc.toByteArray();
    kDebug()<<"content: "<<content;
    zipFile.writeFile( "content.xml", QString(), QString(), content.data(), content.size() );

    foreach( WorksheetEntry* entry, m_entries )
    {
        if ( entry->expression() )
            entry->expression()->saveAdditionalData( &zipFile );
    }

    /*zipFile.close();*/
}

void Worksheet::load(const QString& filename )
{
    // m_file is always local so we can use QFile on it
    KZip file(filename);
    if (!file.open(QIODevice::ReadOnly))
        return ;

    const KArchiveEntry* contentEntry=file.directory()->entry("content.xml");
    if (!contentEntry->isFile())
        kDebug()<<"error";
    const KArchiveFile* content=static_cast<const KArchiveFile*>(contentEntry);
    QByteArray data=content->data();

    kDebug()<<"read: "<<data;

    QDomDocument doc;
    doc.setContent(data);
    QDomElement root=doc.documentElement();
    kDebug()<<root.tagName();

    MathematiK::Backend* b=MathematiK::Backend::createBackend(root.attribute("backend"), parent());
    if (!b)
    {
        KMessageBox::error(this, i18n("The backend with which this file was generated is not installed."), i18n("MathematiK"));
        return;
    }
    m_session=b->createSession();
    m_session->login();
    emit sessionChanged();

    clear();
    m_entries.clear();
    initMainTable();

    kDebug()<<"loading entries";
    QDomElement expressionChild = root.firstChildElement("Expression");
    while (!expressionChild.isNull()) {
        appendEntry();
        WorksheetEntry* entry=m_entries.last();
        entry->commandCell().firstCursorPosition().insertText(expressionChild.firstChildElement("Command").text());
        QDomElement result=expressionChild.firstChildElement("Result");
        if (result.attribute("type") == "text")
            entry->setResult(result.text());
        else if (result.attribute("type") == "image" )
        {
            entry->setResult(""); //Make shure there's space for the image
            const KArchiveEntry* imageEntry=file.directory()->entry(result.attribute("filename"));
            if (imageEntry&&imageEntry->isFile())
            {
                const KArchiveFile* imageFile=static_cast<const KArchiveFile*>(imageEntry);
                QImage image=QImage::fromData(imageFile->data());
                document()->addResource(QTextDocument::ImageResource,
                                      KUrl("mydata://"+imageFile->name()),  QVariant(image));
                QTextImageFormat imageFormat;
                imageFormat.setName("mydata://"+imageFile->name());
                entry->resultCell().firstCursorPosition().insertImage(imageFormat);
            }
        }



        expressionChild = expressionChild.nextSiblingElement("Expression");
    }

}

void Worksheet::gotResult()
{
    MathematiK::Expression* expr=qobject_cast<MathematiK::Expression*>(sender());
    //We're only interested in help results, others are handled by the WorksheetEntry
    if(expr->result()->type()==MathematiK::HelpResult::Type)
    {
        kDebug()<<"showing help";
        QString help=expr->result()->toHtml();
        //Do some basic LaTex replacing
        help.replace(QRegExp("\\\\code\\{([^\\}]*)\\}"), "<b>\\1</b>");
        help.replace(QRegExp("\\$([^\\$])\\$"), "<i>\\1</i>");
        help.replace("\\sage", "<b>Sage</b>");
        emit showHelp(help);
    }
}

void Worksheet::removeEntry(QObject* object)
{
    kDebug()<<"removing entry";
    WorksheetEntry* entry=static_cast<WorksheetEntry*>(sender());
    m_entries.removeAll(entry);
    if(m_entries.isEmpty())
        appendEntry();
}


#include "worksheet.moc"
