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
#include "commandentry.h"
#include "textentry.h"

#include "resultproxy.h"
#include "animationhandler.h"
#include <config-cantor.h>

#include <QEvent>
#include <QKeyEvent>
#include <QTextCursor>
#include <QTimer>
#include <QTextLength>
#include <QFontMetrics>
#include <QSyntaxHighlighter>
#include <QDomDocument>
#include <QXmlQuery>

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

#include "worksheet.h"
#include "settings.h"


Worksheet::Worksheet(Cantor::Backend* backend, QWidget* parent) : KRichTextWidget(parent)
{
    setAcceptRichText(true);
    setWordWrapMode(QTextOption::WordWrap);
    setRichTextSupport( FullTextFormattingSupport
    | FullListSupport
    | SupportAlignment
    | SupportRuleLine
    | SupportFormatPainting );

    m_session=backend->createSession();

    setFont(KGlobalSettings::fixedFont());

    QFontMetrics metrics(document()->defaultFont());
    setTabStopWidth(4*metrics.width(' '));

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
        enableCompletion(Settings::self()->completionDefault());
        enableExpressionNumbering(Settings::self()->expressionNumberingDefault());
#ifdef WITH_EPS
        session()->setTypesettingEnabled(Settings::self()->typesetDefault());
#else
        session()->setTypesettingEnabled(false);
#endif
        m_loginFlag=false;
    }

}

void Worksheet::print( QPrinter* printer )
{
    m_proxy->useHighResolution(true);
    foreach(WorksheetEntry* e, m_entries)
        e->update();


    KTextEdit::print(printer);

    m_proxy->useHighResolution(false);
    foreach(WorksheetEntry* e, m_entries)
        e->update();

}

bool Worksheet::event(QEvent* event)
{
    if (event->type() == QEvent::ShortcutOverride)
    {
        //ignore the shortcuts that are used to trigger valid KActions
        //for the current WorksheetEntry
        WorksheetEntry* entry = currentEntry();
        QKeyEvent* ke = static_cast<QKeyEvent *>( event );
        if (entry && entry->worksheetShortcutOverrideEvent( ke, textCursor() ))
        {
                event->ignore();
                return false;
        }
    }

    return KRichTextWidget::event(event);
}

void Worksheet::setCurrentEntry(WorksheetEntry * entry, bool moveCursor)
{
    if (!entry)
        return;
    bool rt = entry->acceptRichText();
    setActionsEnabled(rt);
    setAcceptRichText(rt);
    m_currentEntry = entry;
    entry->setActive(true, moveCursor);
    ensureCursorVisible();
}

void Worksheet::moveToPreviousEntry()
{
    int index=m_entries.indexOf(currentEntry());
    kDebug()<<"index: "<<index;
    if(index>0)
        setCurrentEntry(m_entries[index-1]);
}

void Worksheet::moveToNextEntry()
{
    int index=m_entries.indexOf(currentEntry());
    kDebug()<<"index: "<<index;
    if(index < m_entries.size() - 1)
        setCurrentEntry(m_entries[index+1]);
}

void Worksheet::keyPressEvent(QKeyEvent* event)
{
    WorksheetEntry *entry = entryAt(textCursor());
    if (entry && !entry->worksheetKeyPressEvent(event, textCursor()))
        KRichTextWidget::keyPressEvent(event);
}

void Worksheet::mousePressEvent(QMouseEvent* event)
{
    kDebug()<<"mousePressEvent";
    const QTextCursor cursor = cursorForPosition(event->pos());
    WorksheetEntry *entry = entryAt(cursor);
    if (entry)
    {
        if (!entry->worksheetMousePressEvent(event, cursor))
            KRichTextWidget::mousePressEvent(event);
        if (entry != m_currentEntry)
            setCurrentEntry(entry);
    }
}

void Worksheet::contextMenuEvent(QContextMenuEvent *event)
{
    kDebug() << "contextMenuEvent";
    const QTextCursor cursor = cursorForPosition(event->pos());
    WorksheetEntry* entry = entryAt(cursor);

    if (entry)
    {
        if (!entry->worksheetContextMenuEvent(event, cursor))
            KRichTextWidget::contextMenuEvent(event);
        if (entry != m_currentEntry)
            setCurrentEntry(entry);
    }
    else
    {
        KMenu* defaultMenu = new KMenu(this);

        if(!isRunning())
            defaultMenu->addAction(KIcon("system-run"),i18n("Evaluate Worksheet"),this,SLOT(evaluate()),0);
        else
            defaultMenu->addAction(KIcon("process-stop"),i18n("Interrupt"),this,SLOT(interrupt()),0);

        defaultMenu ->addSeparator();

        if(m_entries.last()->lastPosition() < cursor.position())
        {
            defaultMenu->addAction(i18n("Append Command Entry"),this,SLOT(appendCommandEntry()),0);
            defaultMenu->addAction(i18n("Append Text Entry"),this,SLOT(appendTextEntry()),0);

        }
        else
        {
            setCurrentEntry(entryNextTo(cursor));
            defaultMenu->addAction(i18n("Insert Command Entry"),this,SLOT(insertCommandEntryBefore()),0);
            defaultMenu->addAction(i18n("Insert Text Entry"),this,SLOT(insertTextEntryBefore()),0);
        }

        defaultMenu->popup(event->globalPos());
    }
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
    kDebug()<<"mouseDoubleClickEvent";
    const QTextCursor cursor = cursorForPosition(event->pos());
    WorksheetEntry *entry = entryAt(cursor);
    if (!entry)
        return;
    KRichTextWidget::mouseDoubleClickEvent(event);
    entry->worksheetMouseDoubleClickEvent(event, textCursor());
    if (entry != m_currentEntry)
        setCurrentEntry(entry);
}

void Worksheet::dragMoveEvent(QDragMoveEvent *event)
{
    const QPoint pos = event->answerRect().topLeft();
    const QTextCursor cursor = cursorForPosition(event->pos());
    WorksheetEntry *entry = entryAt(cursor);
    if (entry && entry->acceptsDrop(cursor))
        event->setAccepted(true);
    else
        event->setAccepted(false);
}

void Worksheet::dropEvent(QDropEvent *event)
{
    const QTextCursor cursor = cursorForPosition(event->pos());
    WorksheetEntry *entry = entryAt(cursor);
    if (entry != m_currentEntry)
        setCurrentEntry(entry, false);
    KRichTextWidget::dropEvent(event);
}

void Worksheet::evaluate()
{
    kDebug()<<"evaluate worksheet";
    foreach(WorksheetEntry* entry, m_entries)
    {
        entry->evaluate(false);
    }

    emit modified();
}

void Worksheet::evaluateCurrentEntry()
{
    kDebug() << "evaluation requested...";
    WorksheetEntry* entry = currentEntry();
    if(!entry)
        return;
    if (!entry->evaluate(true))
        return;
    if(Settings::self()->autoEval())
    {
        QList<WorksheetEntry*>::iterator it=m_entries.begin();
        while((*it)!=entry&&it!=m_entries.end())
            ++it;

        it++;

        for(;it!=m_entries.end();++it)
        {
            //kDebug()<<"evaluate"<<entry->command();
            (*it)->evaluate(false);
        }
        if(!m_entries.last()->isEmpty())
            appendCommandEntry();
        else
            setCurrentEntry(m_entries.last());
    }
    else
    {
        if (entry == m_entries.last())
            appendCommandEntry();
        else
            moveToNextEntry();
    }
    emit modified();
}

bool Worksheet::completionEnabled()
{
    return m_completionEnabled;
}

void Worksheet::showCompletion()
{
    WorksheetEntry* current=currentEntry();
    current->showCompletion();
}

WorksheetEntry* Worksheet::currentEntry()
{
    return m_currentEntry;
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

WorksheetEntry* Worksheet::insertEntryAt(const int type, const QTextCursor& cursor)
{
    WorksheetEntry* entry;

    switch(type)
    {
    case (TextEntry::Type):
        entry = new TextEntry(cursor, this);
    break;
    case (CommandEntry::Type):
        entry = new CommandEntry(cursor, this);
    break;
    default:
        entry = 0;
    }

    return entry;
}

WorksheetEntry* Worksheet::entryNextTo(const QTextCursor& cursor)
{
    WorksheetEntry* entry=0;
    foreach(entry, m_entries)
    {
        if (entry->lastPosition() > cursor.position())
            break;
    }

    return entry;
}


WorksheetEntry* Worksheet::appendEntry(const int type)
{
    QTextCursor cursor=document()->rootFrame()->lastCursorPosition();
    WorksheetEntry* entry = insertEntryAt(type, cursor);
    if (entry)
    {
        kDebug() << "Entry Appended";
        m_entries.append(entry);
        setCurrentEntry(entry);
    }
    return entry;
}

WorksheetEntry* Worksheet::appendCommandEntry()
{
   return appendEntry(CommandEntry::Type);
}

WorksheetEntry* Worksheet::appendTextEntry()
{
   return appendEntry(TextEntry::Type);
}

void Worksheet::appendCommandEntry(const QString& text)
{
    WorksheetEntry* entry=m_entries.last();
    if(!entry->isEmpty())
    {
        entry=appendCommandEntry();
    }

    if (entry)
    {
        setCurrentEntry(entry);
        entry->setContent(text);
        evaluateCurrentEntry();
    }
}

WorksheetEntry* Worksheet::insertEntry(int type)
{
    WorksheetEntry* current=currentEntry();
    if(current)
    {
        int index=m_entries.indexOf(current);
        WorksheetEntry* nextE = entryAt(index+1);

        if(!nextE || nextE->type() != type || !nextE->isEmpty())
        {
            QTextCursor cursor = QTextCursor(document());
            cursor.setPosition(current->lastPosition()+1);
            nextE = insertEntryAt(type, cursor);

            m_entries.insert(index+1, nextE);

        }
        setCurrentEntry(nextE);
        return nextE;
    }

    return 0;
}

WorksheetEntry* Worksheet::insertTextEntry()
{
    return insertEntry(TextEntry::Type);
}

WorksheetEntry* Worksheet::insertCommandEntry()
{
    return insertEntry(CommandEntry::Type);
}

void Worksheet::insertCommandEntry(const QString& text)
{
    WorksheetEntry* entry = insertCommandEntry();
    if(entry&&!text.isNull())
    {
        entry->setContent(text);
        evaluateCurrentEntry();
    }
}

WorksheetEntry* Worksheet::insertEntryBefore(int type)
{
    WorksheetEntry* current=currentEntry();
    if(current)
    {
        int index=m_entries.indexOf(current);
        WorksheetEntry* prevE = entryAt(index-1);

        if(!prevE || prevE->type() != type || !prevE->isEmpty())
        {
            QTextCursor cursor = QTextCursor(document());
            cursor.setPosition(current->firstPosition()-1);
            prevE = insertEntryAt(type, cursor);

            m_entries.insert(index, prevE);

        }

        setCurrentEntry(prevE);
        return prevE;
    }

    return 0;
}

WorksheetEntry* Worksheet::insertTextEntryBefore()
{
    return insertEntryBefore(TextEntry::Type);
}

WorksheetEntry* Worksheet::insertCommandEntryBefore()
{
    return insertEntryBefore(CommandEntry::Type);
}

void Worksheet::interrupt()
{
    m_session->interrupt();
    emit updatePrompt();
}

void Worksheet::interruptCurrentEntryEvaluation()
{
    currentEntry()->interruptEvaluation();
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

void Worksheet::enableCompletion(bool enable)
{
    m_completionEnabled=enable;
}

Cantor::Session* Worksheet::session()
{
    return m_session;
}

bool Worksheet::isRunning()
{
    return m_session->status()==Cantor::Session::Running;
}

QDomDocument Worksheet::toXML(KZip* archive)
{
    QDomDocument doc( "CantorWorksheet" );
    QDomElement root=doc.createElement( "Worksheet" );
    root.setAttribute("backend", m_session->backend()->name());
    doc.appendChild(root);

    foreach( WorksheetEntry* entry, m_entries )
    {
        QDomElement el = entry->toXml(doc, archive);
        root.appendChild( el );
    }
    return doc;
}

void Worksheet::save( const QString& filename )
{
    kDebug()<<"saving to filename";
    KZip zipFile( filename );


    if ( !zipFile.open(QIODevice::WriteOnly) )
    {
        KMessageBox::error( this,  i18n( "Cannot write file %1." , filename ),
                            i18n( "Error - Cantor" ));
        return;
    }

    QByteArray content = toXML(&zipFile).toByteArray();
    kDebug()<<"content: "<<content;
    zipFile.writeFile( "content.xml", QString(), QString(), content.data(), content.size() );

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
    QString commentStartingSeq = "";
    QString commentEndingSeq = "";

    Cantor::Backend * const backend=session()->backend();
    if (backend->extensions().contains("ScriptExtension"))
    {
        Cantor::ScriptExtension* e=dynamic_cast<Cantor::ScriptExtension*>(backend->extension("ScriptExtension"));
        cmdSep=e->commandSeparator();
        commentStartingSeq = e->commentStartingSequence();
        commentEndingSeq = e->commentEndingSequence();
    }

    QTextStream stream(&file);

    foreach(WorksheetEntry * const entry, m_entries)
    {
        const QString& str=entry->toPlain(cmdSep, commentStartingSeq, commentEndingSeq);
        if(!str.isEmpty())
            stream << str + '\n';
    }

    file.close();
}

void Worksheet::saveLatex(const QString& filename,  bool exportImages)
{
    kDebug()<<"exporting to Latex: "<<filename;
    kDebug()<<(exportImages ? "": "Not ")<<"exporting images";
    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly))
    {
        KMessageBox::error(this, i18n("Error saving file %1", filename), i18n("Error - Cantor"));
        return;
    }

    QTextStream stream(&file);
    QXmlQuery query(QXmlQuery::XSLT20);
    kDebug() << toXML().toString();
    query.setFocus(toXML().toString());

    QString stylesheet = KStandardDirs::locate("appdata", "xslt/latex.xsl");
    if (stylesheet.isEmpty())
    {
        KMessageBox::error(this, i18n("Error loading latex.xsl stylesheet"), i18n("Error - Cantor"));
        return;
    }

    query.setQuery(QUrl(stylesheet));
    QString out;
    if (query.evaluateTo(&out))
        stream << out;
    file.close();

//
//     if(exportImages)
//     {
//         foreach( WorksheetEntry* entry, m_entries )
//         {
//             if ( entry->expression() )
//             {
//                 Cantor::Result* result=entry->expression()->result();
//                 if(!result)
//                     continue;
//                 KUrl dest(filename);
//                 dest.setFileName(result->url().fileName());
//                 kDebug()<<"saving image to "<<dest;
//                 if(result->type()==Cantor::ImageResult::Type||result->type()==Cantor::EpsResult::Type)
//                     result->save(dest.toLocalFile());
//             }
//         }
//     }

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
                                            "You will only be able to view this worksheet.", backendName), i18n("Cantor"));

    }

    m_session=b->createSession();
    m_loginFlag=true;
    QTimer::singleShot(0, this, SLOT(loginToSession()));

    //Set the Highlighting, depending on the current state
    //If the session isn't logged in, use the default
    enableHighlighting( m_highlighter!=0 || (m_loginFlag && Settings::highlightDefault()) );

    clear();
    m_entries.clear();

    kDebug()<<"loading entries";
    QDomElement expressionChild = root.firstChildElement();
    WorksheetEntry* entry;
    while (!expressionChild.isNull()) {
        QString tag = expressionChild.tagName();
        if (tag == "Expression")
        {
            entry = appendCommandEntry();
            entry->setContent(expressionChild, file);
        }
        else if (tag == "Text")
        {
            entry = appendTextEntry();
            entry->setContent(expressionChild, file);
        }

        expressionChild = expressionChild.nextSiblingElement();
    }

    emit sessionChanged();
}


void Worksheet::gotResult(Cantor::Expression* expr)
{
    if(expr==0)
        expr=qobject_cast<Cantor::Expression*>(sender());

    if(expr==0)
        return;
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
        appendCommandEntry();
}

void Worksheet::removeCurrentEntry()
{
    kDebug()<<"removing current entry";
    WorksheetEntry* entry=currentEntry();
    if(!entry)
        return;

    int index=m_entries.indexOf(entry);

    int position=entry->firstPosition();
    kDebug()<<position;
    QTextCursor cursor = textCursor();
    cursor.setPosition(position - 1);
    cursor.setPosition(entry->lastPosition() + 1, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();

    delete entry;
    m_entries.removeAll(entry);

    entry = entryAt(index);
    if (!entry)
        entry = entryAt(index + 1);
    if (!entry)
        entry = appendCommandEntry();
    setCurrentEntry(entry);
}


void Worksheet::checkEntriesForSanity()
{
    foreach(WorksheetEntry* e, m_entries)
    {
        e->checkForSanity();
    }
}

bool Worksheet::showExpressionIds()
{
    return m_showExpressionIds;
}

void Worksheet::enableExpressionNumbering(bool enable)
{
    m_showExpressionIds=enable;
    emit updatePrompt();
}

void Worksheet::zoomIn(int range)
{
    KTextEdit::zoomIn(range);

    m_proxy->scale(1+range/10.0); //Scale images for 10%

    foreach(WorksheetEntry* e, m_entries)
        e->update();
}

void Worksheet::zoomOut(int range)
{
    KTextEdit::zoomOut(range);
    m_proxy->scale(1-range/10.0); //Scale images for 10%

    foreach(WorksheetEntry* e, m_entries)
        e->update();
}

ResultProxy* Worksheet::resultProxy()
{
    return m_proxy;
}

#include "worksheet.moc"
