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

#include <kdebug.h>
#include <kzip.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kurl.h>

Worksheet::Worksheet(MathematiK::Backend* backend, QWidget* parent) : QTextEdit(parent)
{
    m_session=backend->createSession();
    m_session->login();

    appendEntry();
}

Worksheet::~Worksheet()
{
    m_session->logout();
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
        }else if ( ke->key() == Qt::Key_Up )
        {
            moveCursor(-1);
            return true;
        }else if ( ke->key() == Qt::Key_Down )
        {
            moveCursor(1);
            return true;
        }else if ( ke->key() == Qt::Key_Backspace)
        {
            WorksheetEntry* current=currentEntry();
            if (current)
                if (textCursor().position()==currentEntry()->startPosition()) return true; //Don't delete any further

        }else if ( ke->key() == Qt::Key_Left)
        {
            WorksheetEntry* current=currentEntry();
            if (current&&textCursor().position()<=currentEntry()->startPosition())
            {
                moveCursor(-1);
                return true;
            }

        }else if ( ke->key() == Qt::Key_Right)
        {
            WorksheetEntry* current=currentEntry();
            if (current&&textCursor().position()>=currentEntry()->resultStartPosition())
            {
                moveCursor(1);
                return true;
            }

        }
    }

    return QTextEdit::event( event );

}

void Worksheet::evaluate()
{
    foreach(WorksheetEntry* entry, m_entries)
    {
        QString cmd=entry->command();

        if(cmd.isEmpty()) continue;

        MathematiK::Expression* expr=m_session->evaluateExpression(cmd);
        connect(expr, SIGNAL(gotResult()), this, SLOT(gotResult()));
        entry->setExpression(expr);
    }
    emit modified();
}

void Worksheet::evaluateCurrentEntry()
{
    kDebug()<<"evaluation requested...";
    //For now lets search for the cell, our cursor is positioned in
    WorksheetEntry* entry=currentEntry();
    if(entry)
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
    }
}

void Worksheet::moveCursor(int direction)
{
    int index=currentEntryIndex()+direction;

    WorksheetEntry* e=m_entries.value(index);
    if( e )
        setTextCursor( e->cmdCursor() );
}

WorksheetEntry* Worksheet::currentEntry()
{
    int curPos=textCursor().position();
    foreach(WorksheetEntry* entry, m_entries)
    {
        if(curPos>=entry->startPosition()&&curPos<=entry->resultStartPosition())
            return entry;
    }

    return 0;
}

int Worksheet::currentEntryIndex()
{
    int i=0;
    int curPos=textCursor().position();
    foreach(WorksheetEntry* entry, m_entries)
    {
        if(curPos>=entry->startPosition()&&curPos<=entry->resultStartPosition())
            return i;
        i++;
    }

    return -1;
}

void Worksheet::appendEntry(const QString& text)
{
    WorksheetEntry* entry=new WorksheetEntry( document()->rootFrame()->lastCursorPosition(),this);
    m_entries.append(entry);
    setTextCursor(entry->cmdCursor());

    ensureCursorVisible();

    if(!text.isNull())
    {
        entry->cmdCursor().insertText(text);
        evaluateCurrentEntry();
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

    kDebug()<<"loading entries";
    QDomElement expressionChild = root.firstChildElement("Expression");
    while (!expressionChild.isNull()) {
        appendEntry();
        WorksheetEntry* entry=m_entries.last();
        entry->cmdCursor().insertText(expressionChild.firstChildElement("Command").text());
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
                entry->resultCursor().insertImage(imageFormat);
                //entry->resultCursor().insertImage(image);
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

#include "worksheet.moc"
