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
#include "lib/extension.h"
#include "lib/expression.h"
#include "lib/result.h"
#include "lib/helpresult.h"
#include "lib/latexresult.h"
#include "lib/epsresult.h"
#include "lib/imageresult.h"
#include "lib/defaulthighlighter.h"

#include "worksheetentry.h"

#include "resultproxy.h"
#include "animationhandler.h"
#include "loadedexpression.h"
#include "resultcontextmenu.h"
#include <config-cantor.h>

#include <QEvent>
#include <QKeyEvent>
#include <QTextCursor>
#include <QTimer>
#include <QTextLength>
#include <QFontMetrics>
#include <QSyntaxHighlighter>

#include <kdebug.h>
#include <kzip.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kurl.h>
#include <kstandardshortcut.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <kglobalsettings.h>
#include <kfiledialog.h>
#include <kmenu.h>

#include "settings.h"

Worksheet::Worksheet(Cantor::Backend* backend, QWidget* parent) : KTextEdit(parent)
{
    m_session=backend->createSession();

    setFont(KGlobalSettings::fixedFont());

    QFontMetrics metrics(document()->defaultFont());
    setTabStopWidth(4*metrics.width(' '));

    appendEntry();

    m_highlighter=0;

    m_proxy=new ResultProxy(document());
    document()->documentLayout()->registerHandler(QTextFormat::ImageObject, new AnimationHandler(document()));

    //postpone login, until everything is set up correctly
    m_loginFlag=true;
    QTimer::singleShot(0, this, SLOT(loginToSession()));
}

Worksheet::~Worksheet()
{
    m_session->logout();
}

void Worksheet::loginToSession()
{
    if(m_loginFlag==true)
    {
        m_session->login();

        enableHighlighting(Settings::self()->highlightDefault());
        enableTabCompletion(Settings::self()->tabCompletionDefault());
        enableExpressionNumbering(Settings::self()->expressionNumberingDefault());
#ifdef WITH_EPS
        session()->setTypesettingEnabled(Settings::self()->typesetDefault());
#else
        session()->setTypesettingEnabled(false);
#endif
        m_loginFlag=false;
    }

}

bool Worksheet::event(QEvent* event)
{
    if (event->type() == QEvent::ShortcutOverride)
    {
        QKeyEvent *e = static_cast<QKeyEvent *>( event );
        //ignore the Shift+Return shortcut, so it can be used as a Shortcut for a KAction
        if (e->modifiers() == Qt::ShiftModifier&&
            (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter))
        {
            e->ignore();
            return false;
        }
    }

    return KTextEdit::event(event);
}

void Worksheet::keyPressEvent(QKeyEvent* event)
{
    //if the textCursor is inside a prompt cell, set it to the next
    //command cell.
    WorksheetEntry* current=currentEntry();
    if(current&&current->isInPromptCell(textCursor()))
       setTextCursor(current->commandCell().firstCursorPosition());

    QTimer::singleShot(0, this, SLOT(moveToClosestValidCursor()));

    if ( event->key() == Qt::Key_Tab &&m_tabCompletionEnabled )
    {
        // special tab handling here
        if (current)
        {
            //get the current line of the entry. If it's empty, do a regular tab(indent),
            //otherwise check for tab completion (if supported by the backend)
            const QString line=current->currentLine(textCursor()).trimmed();

            if(line.isEmpty())
            {
                KTextEdit::keyPressEvent(event);
            }else
            {
                Cantor::TabCompletionObject* tco=m_session->tabCompletionFor(line);
                if(tco)
                    current->setTabCompletion(tco);
            }
        }

    }else
    if ( (event->modifiers()==Qt::ShiftModifier) && (event->key() == Qt::Key_Delete)) //TODO: make configurable
    {
        removeCurrentEntry();
    }else
    if ( (event->modifiers()==Qt::NoModifier) && (event->key() == Qt::Key_Enter ||event->key() == Qt::Key_Return))
    {
        //If the current Entry is showing a popup completion box,
        //complete to the currently selected item, when enter is pressed
        //otherwise, just do a normal Enter(line break)
        WorksheetEntry* entry=currentEntry();
        if(entry&&entry->isShowingCompletionPopup())
            entry->applySelectedTabCompletion();
        else
            KTextEdit::keyPressEvent(event);
    }else
    if ( event->key() == Qt::Key_Left )
    {
        KTextEdit::keyPressEvent(event);
        if(current&&current->isInPromptCell(textCursor())) //The cursor is placed at the prompt column. move it up if possible
        {
            int index=m_entries.indexOf(current);
            kDebug()<<"index: "<<index;
            WorksheetEntry* newEntry;
            if(index>0)
                newEntry=m_entries[index-1];
            else
                newEntry=current;

            setTextCursor(newEntry->commandCell().firstCursorPosition());
        }
    }else if ( event->key() == Qt::Key_Right )
    {
        KTextEdit::keyPressEvent(event);
        if(current&&!current->isInCommandCell(textCursor()))
        {
            int index=m_entries.indexOf(current);

            if(index<m_entries.size()-1)
            {
                setTextCursor(m_entries[index+1]->commandCell().firstCursorPosition());
            }else
            {
                setTextCursor(current->commandCell().lastCursorPosition());
            }
        }else
        {
            //If the cursor is behind the last entry, set it to the last position of the last entry
            current=m_entries.last();
            if(textCursor().position()>current->lastPosition())
                setTextCursor(current->commandCell().lastCursorPosition());
        }
    }else if ( event->key() == Qt::Key_Up )
    {
        if(!current)
        {
            KTextEdit::keyPressEvent(event);
            return;
        }

        //Check if we are in the top line of the command cell,
        //if so, pressing up means we want to go to the previous entry

        if(current->isInCommandCell(textCursor())) //We are in the command cell.
        {
            //get the text written between the current cursor, and the beginning
            //of the commandCell
            QTextCursor c=textCursor();
            c.setPosition(current->commandCell().firstCursorPosition().position(), QTextCursor::KeepAnchor);
            QString txt=c.selectedText();

            if(txt.contains(QChar::ParagraphSeparator)||
               txt.contains(QChar::LineSeparator)||
               txt.contains('\n')) //there's still a newline above the cursor, so move only one line up
            {
                KTextEdit::keyPressEvent(event);
                return;
            }else
            {
                int index=m_entries.indexOf(current);
                kDebug()<<"index: "<<index;
                WorksheetEntry* newEntry;
                if(index>0)
                    newEntry=m_entries[index-1];
                else
                    newEntry=current;

                setTextCursor(newEntry->commandCell().firstCursorPosition());
            }
        }else
        {
            setTextCursor(current->commandCell().firstCursorPosition());
        }
    }else if ( event->key() == Qt::Key_Down )
    {
        if(!current)
        {
            KTextEdit::keyPressEvent(event);
            return;
        }

        //Check if we are in the bottom line of the command cell,
        //if so, pressing down means we want to go to the next entry

        //get the text written between the current cursor, and the end
        //of the commandCell
        QTextCursor c=textCursor();
        c.setPosition(current->commandCell().lastCursorPosition().position(), QTextCursor::KeepAnchor);
        QString txt=c.selectedText();

        //if we're in the command cell and there is still a newline under the cursor, only move one line down
        if(current->isInCommandCell(textCursor())&&
            (txt.contains(QChar::ParagraphSeparator)||
              txt.contains(QChar::LineSeparator)||
              txt.contains('\n')
            )
          )
        {
            KTextEdit::keyPressEvent(event);
            return;
        }else
        {
            //move to the next entry
            int index=m_entries.indexOf(current);
            WorksheetEntry* newEntry;
            if(index<m_entries.size()-1)
                newEntry=m_entries[index+1];
            else
                newEntry=current;

            setTextCursor(newEntry->commandCell().firstCursorPosition());
        }
    }
    else
    {
        KTextEdit::keyPressEvent(event);
        QTimer::singleShot(0, this, SLOT(checkEntriesForSanity()));
    }

}

void Worksheet::contextMenuEvent(QContextMenuEvent *event)
{
    QTextCursor cursorAtMouse=cursorForPosition(event->pos());
    WorksheetEntry* current=entryAt(cursorAtMouse);
    if(current&&current->isInResultCell(cursorAtMouse)&&current->expression()&&current->expression()->result())
    {
        kDebug()<<"context menu in result...";
        KMenu* popup=new ResultContextMenu(current, this);

        QMenu* defaultMenu=mousePopupMenu();
        defaultMenu->setTitle(i18n("Other"));
        popup->addMenu(defaultMenu);

        popup->popup(event->globalPos());

    }else
    {
        QTextCursor oldCursor=textCursor();
        KTextEdit::contextMenuEvent(event);
        if(!currentEntry())
            setTextCursor(oldCursor);
    }
}

void Worksheet::mousePressEvent(QMouseEvent* event)
{
    KTextEdit::mousePressEvent(event);
    moveToClosestValidCursor();
}

void Worksheet::mouseReleaseEvent(QMouseEvent* event)
{
    QTextCursor oldCursor=textCursor();
    KTextEdit::mouseMoveEvent(event);
    if(!currentEntry())
        setTextCursor(oldCursor);
}

void Worksheet::mouseDoubleClickEvent(QMouseEvent* event)
{
    KTextEdit::mousePressEvent(event);
    moveToClosestValidCursor();
}

void Worksheet::evaluate()
{
    foreach(WorksheetEntry* entry, m_entries)
    {
        evaluateEntry(entry);
    }


    emit modified();
}

void Worksheet::evaluateCurrentEntry()
{
    kDebug()<<"evaluation requested...";
    WorksheetEntry* entry=currentEntry();
    if(!entry)
        return;

    if (entry->isInCommandCell(textCursor()))
    {
        evaluateEntry(entry);
    }else if (entry->isInCurrentInformationCell(textCursor()))
    {
        entry->addInformation();
    }
}

void Worksheet::evaluateEntry(WorksheetEntry* entry)
{
    entry->removeContextHelp();

    QString cmd=entry->command();
    kDebug()<<"evaluating: "<<cmd;

    Cantor::Expression* expr;
    if(cmd.isEmpty()) return;

    expr=m_session->evaluateExpression(cmd);
    connect(expr, SIGNAL(gotResult()), this, SLOT(gotResult()));

    entry->setExpression(expr);

    if(!m_entries.last()->isEmpty())
        appendEntry();

    emit modified();
}

WorksheetEntry* Worksheet::currentEntry()
{
    return entryAt(textCursor());
}

WorksheetEntry* Worksheet::entryAt(const QTextCursor& cursor)
{
    foreach(WorksheetEntry* entry, m_entries)
    {
        if(entry->contains(cursor))
            return entry;
    }

    return 0;
}

WorksheetEntry* Worksheet::entryAt(int row)
{
    if(row>=0&&row<m_entries.size())
        return m_entries[row];
    else
        return 0;
}

QTextCursor Worksheet::closestValidCursor(const QTextCursor& cursor)
{
    if(cursor.position()>m_entries.last()->lastPosition())
    {
        QTextCursor c=cursor;
        c.setPosition(m_entries.last()->lastPosition());
        return c;
    }

    foreach(WorksheetEntry* entry, m_entries)
    {
        if(entry->contains(cursor))
        {
            return cursor;
        }
        else if (entry->firstPosition()>cursor.position())
        {
            return entry->commandCell().firstCursorPosition();
        }
    }
}

WorksheetEntry* Worksheet::appendEntry()
{
   QTextCursor cursor=document()->rootFrame()->lastCursorPosition();
   WorksheetEntry* entry=new WorksheetEntry( cursor, this );

   connect(entry, SIGNAL(destroyed(QObject*)), this, SLOT(removeEntry(QObject*)));
   m_entries.append(entry);

   setTextCursor(entry->commandCell().firstCursorPosition());
   ensureCursorVisible();

   return entry;
}

void Worksheet::appendEntry(const QString& text)
{
    WorksheetEntry* oldE=m_entries.last();
    WorksheetEntry* newE=appendEntry();
    WorksheetEntry* target=0;
    if(oldE->isEmpty())
        target=oldE;
    else
        target=newE;

    target->commandCell().firstCursorPosition().insertText(text);
    setTextCursor(target->commandCell().firstCursorPosition());
    ensureCursorVisible();

    evaluateCurrentEntry();

}

WorksheetEntry* Worksheet::insertEntry()
{
    WorksheetEntry* current=currentEntry();
    if(current)
    {
        int index=m_entries.indexOf(current);
        WorksheetEntry* nextE=entryAt(index+1);
        if(!nextE||!nextE->isEmpty())
        {
            QTextCursor c=QTextCursor(document());
            c.setPosition(current->lastPosition()+2);
            WorksheetEntry* entry=new WorksheetEntry(c, this);
            connect(entry, SIGNAL(destroyed(QObject*)), this, SLOT(removeEntry(QObject*)));
            m_entries.insert(index+1, entry);

            return entry;
        }else
        {
            return nextE;
        }
    }

    return 0;
}

void Worksheet::insertEntry(const QString& text)
{
    WorksheetEntry* entry=insertEntry();
    if(entry&&!text.isNull())
    {
        entry->commandCell().firstCursorPosition().insertText(text);
        evaluateCurrentEntry();
    }
}

void Worksheet::interrupt()
{
    m_session->interrupt();
}

void Worksheet::interruptCurrentExpression()
{
    Cantor::Expression* expr=currentEntry()->expression();
    if(expr)
        expr->interrupt();
}

void Worksheet::enableHighlighting(bool highlight)
{
    if(highlight)
    {
        if(m_highlighter)
            m_highlighter->deleteLater();
        m_highlighter=session()->syntaxHighlighter(this);
        if(!m_highlighter)
        {
            m_highlighter=new Cantor::DefaultHighlighter(this);
        }
    }else
    {
        if(m_highlighter)
            m_highlighter->deleteLater();
        m_highlighter=0;
    }
}

void Worksheet::enableTabCompletion(bool enable)
{
    m_tabCompletionEnabled=enable;
}

Cantor::Session* Worksheet::session()
{
    return m_session;
}

bool Worksheet::isRunning()
{
    return m_session->status()==Cantor::Session::Running;
}

void Worksheet::save( const QString& filename )
{
    kDebug()<<"saving to filename";
    KZip zipFile( filename );


    if ( !zipFile.open(QIODevice::WriteOnly) )
    {
        KMessageBox::error( this,  i18n( "Cannot write file %1:\n." , filename ),
                            i18n( "Error - Cantor" ));
        return;
    }

    QDomDocument doc( "CantorWorksheet" );
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

void Worksheet::savePlain(const QString& filename)
{
    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly))
    {
        KMessageBox::error(this, i18n("Error saving file %1", filename), i18n("Error - Cantor"));
        return;
    }

    QString cmdSep=";\n";
    Cantor::Backend * const backend=session()->backend();
    if (backend->extensions().contains("ScriptExtension"))
    {
        Cantor::ScriptExtension* e=dynamic_cast<Cantor::ScriptExtension*>(backend->extension("ScriptExtension"));
        cmdSep=e->commandSeparator();
    }

    QTextStream stream(&file);

    foreach(WorksheetEntry * const entry, m_entries)
    {
        const QString& cmd=entry->command();
        if(!cmd.isEmpty())
            stream<<cmd+cmdSep+'\n';
    }

    file.close();
}

void Worksheet::load(const QString& filename )
{
    // m_file is always local so we can use QFile on it
    KZip file(filename);
    if (!file.open(QIODevice::ReadOnly))
        return ;

    const KArchiveEntry* contentEntry=file.directory()->entry("content.xml");
    if (!contentEntry->isFile())
    {
        kDebug()<<"error";
    }
    const KArchiveFile* content=static_cast<const KArchiveFile*>(contentEntry);
    QByteArray data=content->data();

    kDebug()<<"read: "<<data;

    QDomDocument doc;
    doc.setContent(data);
    QDomElement root=doc.documentElement();
    kDebug()<<root.tagName();

    const QString backendName=root.attribute("backend");
    Cantor::Backend* b=Cantor::Backend::createBackend(backendName);
    if (!b)
    {
        KMessageBox::error(this, i18n("The backend with which this file was generated is not installed. It needs %1", backendName), i18n("Cantor"));
        return;
    }

    if(!b->isEnabled())
    {
        KMessageBox::information(this, i18n("There are some problems with the %1 backend,\n"\
                                            "please check your configuration or install the needed packages.\n"
                                            "You will only be able to view this worksheet", backendName), i18n("Cantor"));

    }

    m_session=b->createSession();
    m_loginFlag=true;
    QTimer::singleShot(0, this, SLOT(loginToSession()));
    emit sessionChanged();

    //Set the Highlighting, depending on the current state
    enableHighlighting(m_highlighter!=0);

    clear();
    m_entries.clear();

    kDebug()<<"loading entries";
    QDomElement expressionChild = root.firstChildElement("Expression");
    while (!expressionChild.isNull()) {
        appendEntry();
        WorksheetEntry* entry=m_entries.last();

        entry->commandCell().firstCursorPosition().insertText(expressionChild.firstChildElement("Command").text());

        LoadedExpression* expr=new LoadedExpression( m_session );
        expr->loadFromXml(expressionChild, file);

        entry->setExpression(expr);

        expressionChild = expressionChild.nextSiblingElement("Expression");
    }

}

void Worksheet::gotResult()
{
    Cantor::Expression* expr=qobject_cast<Cantor::Expression*>(sender());
    //We're only interested in help results, others are handled by the WorksheetEntry
    if(expr->result()->type()==Cantor::HelpResult::Type)
    {
        QString help=expr->result()->toHtml();
        //Do some basic LaTex replacing
        help.replace(QRegExp("\\\\code\\{([^\\}]*)\\}"), "<b>\\1</b>");
        help.replace(QRegExp("\\$([^\\$])\\$"), "<i>\\1</i>");

        emit showHelp(help);
    }
}

void Worksheet::removeEntry(QObject* object)
{
    kDebug()<<"removing entry";
    WorksheetEntry* entry=static_cast<WorksheetEntry*>(object);
    m_entries.removeAll(entry);
    if(m_entries.isEmpty())
        appendEntry();
}

void Worksheet::removeCurrentEntry()
{
    kDebug()<<"removing current entry";
    WorksheetEntry* entry=currentEntry();
    if(!entry)
        return;

    int index=m_entries.indexOf(entry);

    QTextCursor cursor=entry->table()->firstCursorPosition();
    cursor.setPosition(cursor.position()-2);
    cursor.setPosition(entry->table()->lastCursorPosition().position()+1, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();

    m_entries.removeAll(entry);
    int relIndices[]={0, -1, 1};
    kDebug()<<"index: "<<index;
    for(int i=0;i<3;i++)
    {
        kDebug()<<"trying "<<index+relIndices[i];
        entry=entryAt( index+relIndices[i] );
        if(entry)
        {
            kDebug()<<"found at "<<relIndices[i];
            setTextCursor(entry->commandCell().firstCursorPosition());
        }
    }
}

void Worksheet::checkEntriesForSanity()
{
    foreach(WorksheetEntry* e, m_entries)
    {
        e->checkForSanity();
    }
}

void Worksheet::moveToClosestValidCursor()
{
    setTextCursor(closestValidCursor(textCursor()));
}

bool Worksheet::showExpressionIds()
{
    return m_showExpressionIds;
}

void Worksheet::enableExpressionNumbering(bool enable)
{
    m_showExpressionIds=enable;

    foreach(WorksheetEntry* entry, m_entries)
    {
        entry->updatePrompt();
    }
}

void Worksheet::zoomIn(int range)
{
    KTextEdit::zoomIn(range);

    m_proxy->scale(1+range/10.0); //Scale images for 10%

    foreach(WorksheetEntry* e, m_entries)
        e->updateResult();
}

void Worksheet::zoomOut(int range)
{
    KTextEdit::zoomOut(range);
    m_proxy->scale(1-range/10.0); //Scale images for 10%

    foreach(WorksheetEntry* e, m_entries)
        e->updateResult();
}

ResultProxy* Worksheet::resultProxy()
{
    return m_proxy;
}

#include "worksheet.moc"
