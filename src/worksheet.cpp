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

#include "worksheet.h"

#include <QtGlobal>
#include <QApplication>
#include <QBuffer>
#include <QDebug>
#include <QDrag>
#include <QGraphicsWidget>
#include <QPrinter>
#include <QTimer>
#include <QXmlQuery>
#include <QJsonArray>
#include <QJsonDocument>

#include <KMessageBox>
#include <KActionCollection>
#include <KFontAction>
#include <KFontSizeAction>
#include <KToggleAction>
#include <KLocalizedString>

#include "settings.h"
#include "commandentry.h"
#include "textentry.h"
#include "markdownentry.h"
#include "latexentry.h"
#include "imageentry.h"
#include "pagebreakentry.h"
#include "placeholderentry.h"
#include "jupyterutils.h"
#include "lib/backend.h"
#include "lib/extension.h"
#include "lib/helpresult.h"
#include "lib/session.h"
#include "lib/defaulthighlighter.h"
#include "lib/backend.h"

#include <config-cantor.h>

const double Worksheet::LeftMargin = 4;
const double Worksheet::RightMargin = 4;
const double Worksheet::TopMargin = 12;
const double Worksheet::EntryCursorLength = 30;
const double Worksheet::EntryCursorWidth = 2;

Worksheet::Worksheet(Cantor::Backend* backend, QWidget* parent)
    : QGraphicsScene(parent)
{
    m_session = backend->createSession();
    m_highlighter = nullptr;

    m_firstEntry = nullptr;
    m_lastEntry = nullptr;
    m_lastFocusedTextItem = nullptr;
    m_dragEntry = nullptr;
    m_placeholderEntry = nullptr;
    m_viewWidth = 0;
    m_protrusion = 0;
    m_dragScrollTimer = nullptr;

    m_choosenCursorEntry = nullptr;
    m_isCursorEntryAfterLastEntry = false;

    m_entryCursorItem = addLine(0,0,0,0);
    const QColor& color = (palette().color(QPalette::Base).lightness() < 128) ? Qt::white : Qt::black;
    QPen pen(color);
    pen.setWidth(EntryCursorWidth);
    m_entryCursorItem->setPen(pen);
    m_entryCursorItem->hide();

    m_cursorItemTimer = new QTimer(this);
    connect(m_cursorItemTimer, &QTimer::timeout, this, &Worksheet::animateEntryCursor);
    m_cursorItemTimer->start(500);

    m_isPrinting = false;
    m_loginDone = false;
    m_readOnly = false;
    m_isLoadingFromFile = false;

    enableHighlighting(Settings::self()->highlightDefault());
    enableCompletion(Settings::self()->completionDefault());
    enableExpressionNumbering(Settings::self()->expressionNumberingDefault());
    enableAnimations(Settings::self()->animationDefault());
    enableEmbeddedMath(Settings::self()->embeddedMathDefault());
}

Worksheet::~Worksheet()
{
    // This is necessary, because a SeachBar might access firstEntry()
    // while the scene is deleted. Maybe there is a better solution to
    // this problem, but I can't seem to find it.
    m_firstEntry = nullptr;
    if (m_loginDone)
        m_session->logout();
    if (m_session)
    {
        disconnect(m_session, 0, 0, 0);
        if (m_session->status() != Cantor::Session::Disable)
            m_session->logout();
        m_session->deleteLater();
        m_session = nullptr;
    }
}

void Worksheet::loginToSession()
{
    m_session->login();
#ifdef WITH_EPS
    session()->setTypesettingEnabled(Settings::self()->typesetDefault());
#else
    session()->setTypesettingEnabled(false);
#endif
    m_loginDone = true;
}

void Worksheet::print(QPrinter* printer)
{
    m_epsRenderer.useHighResolution(true);
    m_isPrinting = true;
    QRect pageRect = printer->pageRect();
    qreal scale = 1; // todo: find good scale for page size
    // todo: use epsRenderer()->scale() for printing ?
    const qreal width = pageRect.width()/scale;
    const qreal height = pageRect.height()/scale;
    setViewSize(width, height, scale, true);

    QPainter painter(printer);
    painter.scale(scale, scale);
    painter.setRenderHint(QPainter::Antialiasing);
    WorksheetEntry* entry = firstEntry();
    qreal y = TopMargin;

    while (entry) {
        qreal h = 0;
        do {
            if (entry->type() == PageBreakEntry::Type) {
                entry = entry->next();
                break;
            }
            h += entry->size().height();
            entry = entry->next();
        } while (entry && h + entry->size().height() <= height);

        render(&painter, QRectF(0, 0, width, height),
               QRectF(0, y, width, h));
        y += h;
        if (entry)
            printer->newPage();
    }

    //render(&painter);

    painter.end();
    m_isPrinting = false;
    m_epsRenderer.useHighResolution(false);
    m_epsRenderer.setScale(-1);  // force update in next call to setViewSize,
    worksheetView()->updateSceneSize(); // ... which happens in here
}

bool Worksheet::isPrinting()
{
    return m_isPrinting;
}

void Worksheet::setViewSize(qreal w, qreal h, qreal s, bool forceUpdate)
{
    Q_UNUSED(h);

    m_viewWidth = w;
    if (s != m_epsRenderer.scale() || forceUpdate) {
        m_epsRenderer.setScale(s);
        for (WorksheetEntry *entry = firstEntry(); entry; entry = entry->next())
            entry->updateEntry();
    }
    updateLayout();
}

void Worksheet::updateLayout()
{
    bool cursorRectVisible = false;
    bool atEnd = worksheetView()->isAtEnd();
    if (currentTextItem()) {
        QRectF cursorRect = currentTextItem()->sceneCursorRect();
        cursorRectVisible = worksheetView()->isVisible(cursorRect);
    }

    const qreal w = m_viewWidth - LeftMargin - RightMargin;
    qreal y = TopMargin;
    const qreal x = LeftMargin;
    for (WorksheetEntry *entry = firstEntry(); entry; entry = entry->next())
        y += entry->setGeometry(x, y, w);
    setSceneRect(QRectF(0, 0, m_viewWidth + m_protrusion, y));
    if (cursorRectVisible)
        makeVisible(worksheetCursor());
    else if (atEnd)
        worksheetView()->scrollToEnd();
    drawEntryCursor();
}

void Worksheet::updateEntrySize(WorksheetEntry* entry)
{
    bool cursorRectVisible = false;
    bool atEnd = worksheetView()->isAtEnd();
    if (currentTextItem()) {
        QRectF cursorRect = currentTextItem()->sceneCursorRect();
        cursorRectVisible = worksheetView()->isVisible(cursorRect);
    }

    qreal y = entry->y() + entry->size().height();
    for (entry = entry->next(); entry; entry = entry->next()) {
        entry->setY(y);
        y += entry->size().height();
    }
    setSceneRect(QRectF(0, 0, m_viewWidth + m_protrusion, y));
    if (cursorRectVisible)
        makeVisible(worksheetCursor());
    else if (atEnd)
        worksheetView()->scrollToEnd();
    drawEntryCursor();
}

void Worksheet::addProtrusion(qreal width)
{
    if (m_itemProtrusions.contains(width))
        ++m_itemProtrusions[width];
    else
        m_itemProtrusions.insert(width, 1);
    if (width > m_protrusion) {
        m_protrusion = width;
        qreal y = lastEntry()->size().height() + lastEntry()->y();
        setSceneRect(QRectF(0, 0, m_viewWidth + m_protrusion, y));
    }
}

void Worksheet::updateProtrusion(qreal oldWidth, qreal newWidth)
{
    removeProtrusion(oldWidth);
    addProtrusion(newWidth);
}

void Worksheet::removeProtrusion(qreal width)
{
    if (--m_itemProtrusions[width] == 0) {
        m_itemProtrusions.remove(width);
        if (width == m_protrusion) {
            qreal max = -1;
            for (qreal p : m_itemProtrusions.keys()) {
                if (p > max)
                    max = p;
            }
            m_protrusion = max;
            qreal y = lastEntry()->size().height() + lastEntry()->y();
            setSceneRect(QRectF(0, 0, m_viewWidth + m_protrusion, y));
        }
    }
}

bool Worksheet::isEmpty()
{
    return !m_firstEntry;
}

bool Worksheet::isLoadingFromFile()
{
    return m_isLoadingFromFile;
}

void Worksheet::makeVisible(WorksheetEntry* entry)
{
    QRectF r = entry->boundingRect();
    r = entry->mapRectToScene(r);
    r.adjust(0, -10, 0, 10);
    worksheetView()->makeVisible(r);
}

void Worksheet::makeVisible(const WorksheetCursor& cursor)
{
    if (cursor.textCursor().isNull()) {
        if (cursor.entry())
            makeVisible(cursor.entry());
        return;
    }
    QRectF r = cursor.textItem()->sceneCursorRect(cursor.textCursor());
    QRectF er = cursor.entry()->boundingRect();
    er = cursor.entry()->mapRectToScene(er);
    er.adjust(0, -10, 0, 10);
    r.adjust(0, qMax(qreal(-100.0), er.top() - r.top()),
             0, qMin(qreal(100.0), er.bottom() - r.bottom()));
    worksheetView()->makeVisible(r);
}

WorksheetView* Worksheet::worksheetView()
{
    return qobject_cast<WorksheetView*>(views().first());
}

void Worksheet::setModified()
{
    emit modified();
}

WorksheetCursor Worksheet::worksheetCursor()
{
    WorksheetEntry* entry = currentEntry();
    WorksheetTextItem* item = currentTextItem();

    if (!entry || !item)
        return WorksheetCursor();
    return WorksheetCursor(entry, item, item->textCursor());
}

void Worksheet::setWorksheetCursor(const WorksheetCursor& cursor)
{
    if (!cursor.isValid())
        return;

    if (m_lastFocusedTextItem)
        m_lastFocusedTextItem->clearSelection();

    m_lastFocusedTextItem = cursor.textItem();

    cursor.textItem()->setTextCursor(cursor.textCursor());
}

WorksheetEntry* Worksheet::currentEntry()
{
    QGraphicsItem* item = focusItem();
    // Entry cursor activate
    if (m_choosenCursorEntry || m_isCursorEntryAfterLastEntry)
        return nullptr;

    if (!item /*&& !hasFocus()*/)
        item = m_lastFocusedTextItem;
    /*else
      m_focusItem = item;*/
    while (item && (item->type() < QGraphicsItem::UserType ||
                    item->type() >= QGraphicsItem::UserType + 100))
        item = item->parentItem();
    if (item) {
        WorksheetEntry* entry = qobject_cast<WorksheetEntry*>(item->toGraphicsObject());
        if (entry && entry->aboutToBeRemoved()) {
            if (entry->isAncestorOf(m_lastFocusedTextItem))
                m_lastFocusedTextItem = nullptr;
            return nullptr;
        }
        return entry;
    }
    return nullptr;
}

WorksheetEntry* Worksheet::firstEntry()
{
    return m_firstEntry;
}

WorksheetEntry* Worksheet::lastEntry()
{
    return m_lastEntry;
}

void Worksheet::setFirstEntry(WorksheetEntry* entry)
{
    if (m_firstEntry)
        disconnect(m_firstEntry, SIGNAL(aboutToBeDeleted()),
                   this, SLOT(invalidateFirstEntry()));
    m_firstEntry = entry;
    if (m_firstEntry)
        connect(m_firstEntry, SIGNAL(aboutToBeDeleted()),
                this, SLOT(invalidateFirstEntry()), Qt::DirectConnection);
}

void Worksheet::setLastEntry(WorksheetEntry* entry)
{
    if (m_lastEntry)
        disconnect(m_lastEntry, SIGNAL(aboutToBeDeleted()),
                   this, SLOT(invalidateLastEntry()));
    m_lastEntry = entry;
    if (m_lastEntry)
        connect(m_lastEntry, SIGNAL(aboutToBeDeleted()),
                this, SLOT(invalidateLastEntry()), Qt::DirectConnection);
}

void Worksheet::invalidateFirstEntry()
{
    if (m_firstEntry)
        setFirstEntry(m_firstEntry->next());
}

void Worksheet::invalidateLastEntry()
{
    if (m_lastEntry)
        setLastEntry(m_lastEntry->previous());
}

WorksheetEntry* Worksheet::entryAt(qreal x, qreal y)
{
    QGraphicsItem* item = itemAt(x, y, QTransform());
    while (item && (item->type() <= QGraphicsItem::UserType ||
                    item->type() >= QGraphicsItem::UserType + 100))
        item = item->parentItem();
    if (item)
        return qobject_cast<WorksheetEntry*>(item->toGraphicsObject());
    return nullptr;
}

WorksheetEntry* Worksheet::entryAt(QPointF p)
{
    return entryAt(p.x(), p.y());
}

void Worksheet::focusEntry(WorksheetEntry *entry)
{
    if (!entry)
        return;
    entry->focusEntry();
    resetEntryCursor();
    //bool rt = entry->acceptRichText();
    //setActionsEnabled(rt);
    //setAcceptRichText(rt);
    //ensureCursorVisible();
}

void Worksheet::startDrag(WorksheetEntry* entry, QDrag* drag)
{
    if (m_readOnly)
        return;

    resetEntryCursor();
    m_dragEntry = entry;
    WorksheetEntry* prev = entry->previous();
    WorksheetEntry* next = entry->next();
    m_placeholderEntry = new PlaceHolderEntry(this, entry->size());
    m_placeholderEntry->setPrevious(prev);
    m_placeholderEntry->setNext(next);
    if (prev)
        prev->setNext(m_placeholderEntry);
    else
        setFirstEntry(m_placeholderEntry);
    if (next)
        next->setPrevious(m_placeholderEntry);
    else
        setLastEntry(m_placeholderEntry);
    m_dragEntry->hide();
    Qt::DropAction action = drag->exec();

    qDebug() << action;
    if (action == Qt::MoveAction && m_placeholderEntry) {
        qDebug() << "insert in new position";
        prev = m_placeholderEntry->previous();
        next = m_placeholderEntry->next();
    }
    m_dragEntry->setPrevious(prev);
    m_dragEntry->setNext(next);
    if (prev)
        prev->setNext(m_dragEntry);
    else
        setFirstEntry(m_dragEntry);
    if (next)
        next->setPrevious(m_dragEntry);
    else
        setLastEntry(m_dragEntry);
    m_dragEntry->show();
    m_dragEntry->focusEntry();
    const QPointF scenePos = worksheetView()->sceneCursorPos();
    if (entryAt(scenePos) != m_dragEntry)
        m_dragEntry->hideActionBar();
    updateLayout();
    if (m_placeholderEntry) {
        m_placeholderEntry->setPrevious(nullptr);
        m_placeholderEntry->setNext(nullptr);
        m_placeholderEntry->hide();
        m_placeholderEntry->deleteLater();
        m_placeholderEntry = nullptr;
    }
    m_dragEntry = nullptr;
}

void Worksheet::evaluate()
{
    qDebug()<<"evaluate worksheet";
    if (!m_loginDone && !m_readOnly)
        loginToSession();

    firstEntry()->evaluate(WorksheetEntry::EvaluateNext);

    emit modified();
}

void Worksheet::evaluateCurrentEntry()
{
    if (!m_loginDone && !m_readOnly)
        loginToSession();

    WorksheetEntry* entry = currentEntry();
    if(!entry)
        return;
    entry->evaluateCurrentItem();
}

bool Worksheet::completionEnabled()
{
    return m_completionEnabled;
}

void Worksheet::showCompletion()
{
    WorksheetEntry* current = currentEntry();
    if (current)
        current->showCompletion();
}

WorksheetEntry* Worksheet::appendEntry(const int type, bool focus)
{
    WorksheetEntry* entry = WorksheetEntry::create(type, this);

    if (entry)
    {
        qDebug() << "Entry Appended";
        entry->setPrevious(lastEntry());
        if (lastEntry())
            lastEntry()->setNext(entry);
        if (!firstEntry())
            setFirstEntry(entry);
        setLastEntry(entry);
        updateLayout();
        if (focus)
        {
            makeVisible(entry);
            focusEntry(entry);
        }
        emit modified();
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

WorksheetEntry* Worksheet::appendMarkdownEntry()
{
   return appendEntry(MarkdownEntry::Type);
}

WorksheetEntry* Worksheet::appendPageBreakEntry()
{
    return appendEntry(PageBreakEntry::Type);
}

WorksheetEntry* Worksheet::appendImageEntry()
{
   return appendEntry(ImageEntry::Type);
}

WorksheetEntry* Worksheet::appendLatexEntry()
{
    return appendEntry(LatexEntry::Type);
}

void Worksheet::appendCommandEntry(const QString& text)
{
    WorksheetEntry* entry = lastEntry();
    if(!entry->isEmpty())
    {
        entry = appendCommandEntry();
    }

    if (entry)
    {
        focusEntry(entry);
        entry->setContent(text);
        evaluateCurrentEntry();
    }
}

WorksheetEntry* Worksheet::insertEntry(const int type, WorksheetEntry* current)
{
    if (!current)
        current = currentEntry();

    if (!current)
        return appendEntry(type);

    WorksheetEntry *next = current->next();
    WorksheetEntry *entry = nullptr;

    if (!next || next->type() != type || !next->isEmpty())
    {
        entry = WorksheetEntry::create(type, this);
        entry->setPrevious(current);
        entry->setNext(next);
        current->setNext(entry);
        if (next)
            next->setPrevious(entry);
        else
            setLastEntry(entry);
        updateLayout();
        emit modified();
    } else {
        entry = next;
    }

    focusEntry(entry);
    makeVisible(entry);
    return entry;
}

WorksheetEntry* Worksheet::insertTextEntry(WorksheetEntry* current)
{
    return insertEntry(TextEntry::Type, current);
}

WorksheetEntry* Worksheet::insertMarkdownEntry(WorksheetEntry* current)
{
    return insertEntry(MarkdownEntry::Type, current);
}

WorksheetEntry* Worksheet::insertCommandEntry(WorksheetEntry* current)
{
    return insertEntry(CommandEntry::Type, current);
}

WorksheetEntry* Worksheet::insertImageEntry(WorksheetEntry* current)
{
    return insertEntry(ImageEntry::Type, current);
}

WorksheetEntry* Worksheet::insertPageBreakEntry(WorksheetEntry* current)
{
    return insertEntry(PageBreakEntry::Type, current);
}

WorksheetEntry* Worksheet::insertLatexEntry(WorksheetEntry* current)
{
    return insertEntry(LatexEntry::Type, current);
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

WorksheetEntry* Worksheet::insertEntryBefore(int type, WorksheetEntry* current)
{
    if (!current)
        current = currentEntry();

    if (!current)
        return nullptr;

    WorksheetEntry *prev = current->previous();
    WorksheetEntry *entry = nullptr;

    if(!prev || prev->type() != type || !prev->isEmpty())
    {
        entry = WorksheetEntry::create(type, this);
        entry->setNext(current);
        entry->setPrevious(prev);
        current->setPrevious(entry);
        if (prev)
            prev->setNext(entry);
        else
            setFirstEntry(entry);
        updateLayout();
        emit modified();
    }
    else
        entry = prev;

    focusEntry(entry);
    return entry;
}

WorksheetEntry* Worksheet::insertTextEntryBefore(WorksheetEntry* current)
{
    return insertEntryBefore(TextEntry::Type, current);
}

WorksheetEntry* Worksheet::insertMarkdownEntryBefore(WorksheetEntry* current)
{
    return insertEntryBefore(MarkdownEntry::Type, current);
}

WorksheetEntry* Worksheet::insertCommandEntryBefore(WorksheetEntry* current)
{
    return insertEntryBefore(CommandEntry::Type, current);
}

WorksheetEntry* Worksheet::insertPageBreakEntryBefore(WorksheetEntry* current)
{
    return insertEntryBefore(PageBreakEntry::Type, current);
}

WorksheetEntry* Worksheet::insertImageEntryBefore(WorksheetEntry* current)
{
    return insertEntryBefore(ImageEntry::Type, current);
}

WorksheetEntry* Worksheet::insertLatexEntryBefore(WorksheetEntry* current)
{
    return insertEntryBefore(LatexEntry::Type, current);
}

void Worksheet::interrupt()
{
    if (m_session->status() == Cantor::Session::Running)
    {
        m_session->interrupt();
        emit updatePrompt();
    }
}

void Worksheet::interruptCurrentEntryEvaluation()
{
    currentEntry()->interruptEvaluation();
}

void Worksheet::highlightItem(WorksheetTextItem* item)
{
    if (!m_highlighter)
        return;

    QTextDocument *oldDocument = m_highlighter->document();
    QList<QList<QTextLayout::FormatRange> > formats;

    if (oldDocument)
    {
        for (QTextBlock b = oldDocument->firstBlock();
             b.isValid(); b = b.next())
        {
            formats.append(b.layout()->additionalFormats());
        }
    }

    // Not every highlighter is a Cantor::DefaultHighligther (e.g. the
    // highlighter for KAlgebra)
    Cantor::DefaultHighlighter* hl = qobject_cast<Cantor::DefaultHighlighter*>(m_highlighter);
    if (hl) {
        hl->setTextItem(item);
    } else {
        m_highlighter->setDocument(item->document());
    }

    if (oldDocument)
    {
        QTextCursor cursor(oldDocument);
        cursor.beginEditBlock();
        for (QTextBlock b = oldDocument->firstBlock();
             b.isValid(); b = b.next())
        {
            b.layout()->setAdditionalFormats(formats.first());
            formats.pop_front();
        }
        cursor.endEditBlock();
    }

}

void Worksheet::rehighlight()
{
    if(m_highlighter)
    {
        // highlight every entry
        WorksheetEntry* entry;
        for (entry = firstEntry(); entry; entry = entry->next()) {
            WorksheetTextItem* item = entry->highlightItem();
            if (!item)
                continue;
            highlightItem(item);
            m_highlighter->rehighlight();
        }
        entry = currentEntry();
        WorksheetTextItem* textitem = entry ? entry->highlightItem() : nullptr;
        if (textitem && textitem->hasFocus())
            highlightItem(textitem);
    } else
    {
        // remove highlighting from entries
        WorksheetEntry* entry;
        for (entry = firstEntry(); entry; entry = entry->next()) {
            WorksheetTextItem* item = entry->highlightItem();
            if (!item)
                continue;
            QTextCursor cursor(item->document());
            cursor.beginEditBlock();
            for (QTextBlock b = item->document()->firstBlock();
                 b.isValid(); b = b.next())
            {
                b.layout()->clearAdditionalFormats();
            }
            cursor.endEditBlock();
        }
        update();
    }
}

void Worksheet::enableHighlighting(bool highlight)
{
    if(highlight)
    {
        if(m_highlighter)
            m_highlighter->deleteLater();

        if (!m_readOnly)
            m_highlighter=session()->syntaxHighlighter(this);
        else
            m_highlighter=nullptr;

        if(!m_highlighter)
            m_highlighter=new Cantor::DefaultHighlighter(this);

        connect(m_highlighter, SIGNAL(rulesChanged()), this, SLOT(rehighlight()));

    }else
    {
        if(m_highlighter)
            m_highlighter->deleteLater();
        m_highlighter=nullptr;
    }

    rehighlight();
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
    return m_session && m_session->status()==Cantor::Session::Running;
}

bool Worksheet::isReadOnly()
{
    return m_readOnly;
}

bool Worksheet::showExpressionIds()
{
    return m_showExpressionIds;
}

bool Worksheet::animationsEnabled()
{
    return m_animationsEnabled;
}

void Worksheet::enableAnimations(bool enable)
{
    m_animationsEnabled = enable;
}

bool Worksheet::embeddedMathEnabled()
{
    return m_embeddedMathEnabled;
}

void Worksheet::enableEmbeddedMath(bool enable)
{
    m_embeddedMathEnabled = enable;
}

void Worksheet::enableExpressionNumbering(bool enable)
{
    m_showExpressionIds=enable;
    emit updatePrompt();
}

QDomDocument Worksheet::toXML(KZip* archive)
{
    QDomDocument doc( QLatin1String("CantorWorksheet") );
    QDomElement root=doc.createElement( QLatin1String("Worksheet") );
    root.setAttribute(QLatin1String("backend"), (m_session ? m_session->backend()->name(): m_backendName));
    doc.appendChild(root);

    for( WorksheetEntry* entry = firstEntry(); entry; entry = entry->next())
    {
        QDomElement el = entry->toXml(doc, archive);
        root.appendChild( el );
    }
    return doc;
}

QJsonDocument Worksheet::toJupyterJson()
{
    QJsonDocument doc;
    QJsonObject root;

    QJsonObject metadata;

    QJsonObject kernalInfo;
    if (m_session && m_session->backend())
        kernalInfo = JupyterUtils::getKernelspec(m_session->backend());
    else
        kernalInfo.insert(QLatin1String("name"), m_backendName);
    metadata.insert(QLatin1String("kernelspec"), kernalInfo);

    root.insert(QLatin1String("metadata"), metadata);

    root.insert(QLatin1String("nbformat"), 4);
    root.insert(QLatin1String("nbformat_minor"), 1);

    QJsonArray cells;
    for( WorksheetEntry* entry = firstEntry(); entry; entry = entry->next())
    {
        const QJsonValue entryJson = entry->toJupyterJson();

        if (!entryJson.isNull())
            cells.append(entryJson);
    }
    root.insert(QLatin1String("cells"), cells);

    doc.setObject(root);
    return doc;
}

void Worksheet::save( const QString& filename )
{
    QFile file(filename);
    if ( !file.open(QIODevice::WriteOnly) )
    {
        KMessageBox::error( worksheetView(),
                            i18n( "Cannot write file %1." , filename ),
                            i18n( "Error - Cantor" ));
        return;
    }

    save(&file);
}

QByteArray Worksheet::saveToByteArray()
{
    QBuffer buffer;
    save(&buffer);

    return buffer.buffer();
}

void Worksheet::save( QIODevice* device)
{
    qDebug()<<"saving to filename";
    switch (m_type)
    {
        case CantorWorksheet:
        {
            KZip zipFile( device );

            if ( !zipFile.open(QIODevice::WriteOnly) )
            {
                KMessageBox::error( worksheetView(),
                                    i18n( "Cannot write file." ),
                                    i18n( "Error - Cantor" ));
                return;
            }

            QByteArray content = toXML(&zipFile).toByteArray();
            qDebug()<<"content: "<<content;
            zipFile.writeFile( QLatin1String("content.xml"), content.data());
            break;
        }

        case JupyterNotebook:
        {
            if (!device->isWritable())
            {
                KMessageBox::error( worksheetView(),
                                    i18n( "Cannot write file." ),
                                    i18n( "Error - Cantor" ));
                return;
            }

            const QJsonDocument& doc = toJupyterJson();
            device->write(doc.toJson(QJsonDocument::Indented));
            break;
        }
    }
}


void Worksheet::savePlain(const QString& filename)
{
    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly))
    {
        KMessageBox::error(worksheetView(), i18n("Error saving file %1", filename), i18n("Error - Cantor"));
        return;
    }

    QString cmdSep=QLatin1String(";\n");
    QString commentStartingSeq = QLatin1String("");
    QString commentEndingSeq = QLatin1String("");

    if (!m_readOnly)
    {
        Cantor::Backend * const backend=session()->backend();
        if (backend->extensions().contains(QLatin1String("ScriptExtension")))
        {
            Cantor::ScriptExtension* e=dynamic_cast<Cantor::ScriptExtension*>(backend->extension(QLatin1String(("ScriptExtension"))));
            cmdSep=e->commandSeparator();
            commentStartingSeq = e->commentStartingSequence();
            commentEndingSeq = e->commentEndingSequence();
        }
    }
    else
        KMessageBox::information(worksheetView(), i18n("In read-only mode Cantor couldn't guarantee, that the export will be valid for %1", m_backendName), i18n("Cantor"));

    QTextStream stream(&file);

    for(WorksheetEntry * entry = firstEntry(); entry; entry = entry->next())
    {
        const QString& str=entry->toPlain(cmdSep, commentStartingSeq, commentEndingSeq);
        if(!str.isEmpty())
            stream << str + QLatin1Char('\n');
    }

    file.close();
}

void Worksheet::saveLatex(const QString& filename)
{
    qDebug()<<"exporting to Latex: " <<filename;

    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly))
    {
        KMessageBox::error(worksheetView(), i18n("Error saving file %1", filename), i18n("Error - Cantor"));
        return;
    }

    QString xml = toXML().toString();
    QTextStream stream(&file);
    QXmlQuery query(QXmlQuery::XSLT20);
    query.setFocus(xml);

    QString stylesheet = QStandardPaths::locate(QStandardPaths::DataLocation, QLatin1String("xslt/latex.xsl"));
    if (stylesheet.isEmpty())
    {
        KMessageBox::error(worksheetView(), i18n("Error loading latex.xsl stylesheet"), i18n("Error - Cantor"));
        return;
    }

    query.setQuery(QUrl(stylesheet));
    QString out;
    if (query.evaluateTo(&out))
        // Transform HTML escaped special characters to valid LaTeX characters (&, <, >)
        stream << out.replace(QLatin1String("&amp;"), QLatin1String("&"))
                     .replace(QLatin1String("&gt;"), QLatin1String(">"))
                     .replace(QLatin1String("&lt;"), QLatin1String("<"));
    file.close();
}

bool Worksheet::load(const QString& filename )
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        KMessageBox::error(worksheetView(), i18n("Couldn't open the file %1", filename), i18n("Cantor"));
        return false;
    }

    bool rc = load(&file);
    if (rc && !m_readOnly)
        m_session->setWorksheetPath(filename);

    return rc;
}

void Worksheet::load(QByteArray* data)
{
    QBuffer buf(data);
    load(&buf);
}

bool Worksheet::load(QIODevice* device)
{
    if (!device->isReadable())
    {
        KMessageBox::error(worksheetView(), i18n("Couldn't open the selected file for reading"), i18n("Cantor"));
        return false;
    }

    KZip archive(device);

    if (archive.open(QIODevice::ReadOnly))
        return loadCantorWorksheet(archive);
    else
    {
        qDebug() <<"not a zip file";
        // Go to begin of data, we need read all data in second time
        device->seek(0);

        QJsonParseError error;
        const QJsonDocument& doc = QJsonDocument::fromJson(device->readAll(), &error);
        if (error.error != QJsonParseError::NoError)
        {
            qDebug()<<"not a json file, parsing failed with error: " << error.errorString();
            QApplication::restoreOverrideCursor();
            KMessageBox::error(worksheetView(), i18n("The selected file is not a valid Cantor or Jupyter project file."), i18n("Cantor"));
            return false;
        }
        else
            return loadJupyterNotebook(doc);
    }
}

bool Worksheet::loadCantorWorksheet(const KZip& archive)
{
    m_type = Type::CantorWorksheet;

    const KArchiveEntry* contentEntry=archive.directory()->entry(QLatin1String("content.xml"));
    if (!contentEntry->isFile())
    {
        qDebug()<<"content.xml file not found in the zip archive";
        QApplication::restoreOverrideCursor();
        KMessageBox::error(worksheetView(), i18n("The selected file is not a valid Cantor project file."), i18n("Cantor"));
        return false;
    }

    const KArchiveFile* content = static_cast<const KArchiveFile*>(contentEntry);
    QByteArray data = content->data();

//     qDebug()<<"read: "<<data;

    QDomDocument doc;
    doc.setContent(data);
    QDomElement root=doc.documentElement();
//     qDebug()<<root.tagName();

    m_backendName=root.attribute(QLatin1String("backend"));
    Cantor::Backend* b=Cantor::Backend::getBackend(m_backendName);
    if (!b)
    {
        QApplication::restoreOverrideCursor();
        KMessageBox::information(worksheetView(), i18n("%1 backend was not found. Editing and executing entries is not possible", m_backendName), i18n("Cantor"));
        m_readOnly = true;
    }
    else
        m_readOnly = false;

    if(!m_readOnly && !b->isEnabled())
    {
        QApplication::restoreOverrideCursor();
        KMessageBox::information(worksheetView(), i18n("There are some problems with the %1 backend,\n"\
                                            "please check your configuration or install the needed packages.\n"
                                            "You will only be able to view this worksheet.", m_backendName), i18n("Cantor"));
        m_readOnly = true;
    }

    if (m_readOnly)
    {
        // TODO: Handle this here?
        for (QAction* action : m_richTextActionList)
            action->setEnabled(false);
    }

    m_isLoadingFromFile = true;

    //cleanup the worksheet and all it contains
    delete m_session;
    m_session=nullptr;
    m_loginDone = false;

    //file can only be loaded in a worksheet that was not eidted/modified yet (s.a. CantorShell::load())
    //in this case on the default "first entry" is available -> delete it.
    if (m_firstEntry) {
        delete m_firstEntry;
        m_firstEntry = nullptr;
    }

    resetEntryCursor();

    if (!m_readOnly)
        m_session=b->createSession();

    qDebug()<<"loading entries";
    QDomElement expressionChild = root.firstChildElement();
    WorksheetEntry* entry = nullptr;
    while (!expressionChild.isNull()) {
        QString tag = expressionChild.tagName();
        // Don't add focus on load
        if (tag == QLatin1String("Expression"))
        {
            entry = appendEntry(CommandEntry::Type, false);
            entry->setContent(expressionChild, archive);
        } else if (tag == QLatin1String("Text"))
        {
            entry = appendEntry(TextEntry::Type, false);
            entry->setContent(expressionChild, archive);
        } else if (tag == QLatin1String("Markdown"))
        {
            entry = appendEntry(MarkdownEntry::Type, false);
            entry->setContent(expressionChild, archive);
        } else if (tag == QLatin1String("Latex"))
        {
            entry = appendEntry(LatexEntry::Type, false);
            entry->setContent(expressionChild, archive);
        } else if (tag == QLatin1String("PageBreak"))
        {
            entry = appendEntry(PageBreakEntry::Type, false);
            entry->setContent(expressionChild, archive);
        }
        else if (tag == QLatin1String("Image"))
        {
            entry = appendEntry(ImageEntry::Type, false);
            entry->setContent(expressionChild, archive);
        }

        if (m_readOnly && entry)
        {
            entry->setAcceptHoverEvents(false);
            entry = nullptr;
        }

        expressionChild = expressionChild.nextSiblingElement();
    }

    if (m_readOnly)
        clearFocus();

    m_isLoadingFromFile = false;

    //Set the Highlighting, depending on the current state
    //If the session isn't logged in, use the default
    enableHighlighting( m_highlighter!=nullptr || Settings::highlightDefault() );

    emit loaded();
    return true;
}

bool Worksheet::loadJupyterNotebook(const QJsonDocument& doc)
{
    m_type = Type::JupyterNotebook;


    if (!JupyterUtils::isJupyterNotebook(doc))
    {
        QApplication::restoreOverrideCursor();
        showInvalidNotebookSchemeError();
        return false;
    }

    QJsonObject notebookObject = doc.object();
    int nbformatMajor, nbformatMinor;
    std::tie(nbformatMajor, nbformatMinor) = JupyterUtils::getNbformatVersion(notebookObject);

    if (QT_VERSION_CHECK(nbformatMajor, nbformatMinor, 0) < QT_VERSION_CHECK(4,1,0))
    {
        QApplication::restoreOverrideCursor();
        KMessageBox::error(worksheetView(), i18n("Cantor doesn't support import Jupyter notebooks with version lower that 4.1 (detected %1.%2)",nbformatMajor, nbformatMinor), i18n("Cantor"));
        return false;
    }

    const QJsonArray& cells = JupyterUtils::getCells(notebookObject);
    const QJsonObject& metadata = JupyterUtils::getMetadata(notebookObject);

    const QJsonObject& kernalspec = metadata.value(QLatin1String("kernelspec")).toObject();
    m_backendName = JupyterUtils::getKernelName(kernalspec);
    if (kernalspec.isEmpty() || m_backendName.isEmpty())
    {
        QApplication::restoreOverrideCursor();
        showInvalidNotebookSchemeError();
        return false;
    }

    Cantor::Backend* backend = Cantor::Backend::getBackend(m_backendName);
    if (!backend)
    {
        QApplication::restoreOverrideCursor();
        KMessageBox::information(worksheetView(), i18n("%1 backend was not found. Editing and executing entries is not possible", m_backendName), i18n("Cantor"));
        m_readOnly = true;
    }
    else
        m_readOnly = false;

    if(!m_readOnly && !backend->isEnabled())
    {
        QApplication::restoreOverrideCursor();
        KMessageBox::information(worksheetView(), i18n("There are some problems with the %1 backend,\n"\
                                            "please check your configuration or install the needed packages.\n"
                                            "You will only be able to view this worksheet.", m_backendName), i18n("Cantor"));
        m_readOnly = true;
    }

    if (m_readOnly)
    {
        // Jupyter TODO: Handle this here? Again?
        for (QAction* action : m_richTextActionList)
            action->setEnabled(false);
    }


    m_isLoadingFromFile = true;

    if (m_session)
        delete m_session;
    m_session = nullptr;
    m_loginDone = false;

    if (m_firstEntry) {
        delete m_firstEntry;
        m_firstEntry = nullptr;
    }

    resetEntryCursor();

    if (!m_readOnly)
        m_session=backend->createSession();

    qDebug() << "loading jupyter entries";

    // Jupyter TODO: handle error, like no object, no 'cell_type', etc
    // Maybe forward back to cantor shell and UI message about failed
    WorksheetEntry* entry = nullptr;
    for (QJsonArray::const_iterator iter = cells.begin(); iter != cells.end(); iter++) {
        if (!JupyterUtils::isJupyterCell(*iter))
            continue;

        const QJsonObject& cell = iter->toObject();
        QString cellType = JupyterUtils::getCellType(cell);

        if (cellType == QLatin1String("code"))
        {
            if (LatexEntry::isConvertableToLatexEntry(cell))
            {
                entry = appendEntry(LatexEntry::Type, false);
                entry->setContentFromJupyter(cell);
                entry->evaluate(WorksheetEntry::InternalEvaluation);
            }
            else
            {
                entry = appendEntry(CommandEntry::Type, false);
                entry->setContentFromJupyter(cell);
            }
        }
        else if (cellType == QLatin1String("markdown"))
        {
            if (TextEntry::isConvertableToTextEntry(cell))
            {
                entry = appendEntry(TextEntry::Type, false);
                entry->setContentFromJupyter(cell);
            }
            else
            {
                // Jupyter TODO: improve finding $$...$$, current realization don't support escaping
                entry = appendEntry(MarkdownEntry::Type, false);
                entry->setContentFromJupyter(cell);
                entry->evaluate(WorksheetEntry::InternalEvaluation);
            }
        }
        else if (cellType == QLatin1String("raw"))
        {
            entry = appendEntry(TextEntry::Type, false);
            entry->setContentFromJupyter(cell);
        }

        if (m_readOnly && entry)
        {
            entry->setAcceptHoverEvents(false);
            entry = nullptr;
        }
    }

    if (m_readOnly)
        clearFocus();

    m_isLoadingFromFile = false;

    enableHighlighting( m_highlighter!=nullptr || Settings::highlightDefault() );

    emit loaded();
    return true;
}

void Worksheet::showInvalidNotebookSchemeError()
{
    KMessageBox::error(worksheetView(), i18n("The file is not a valid Jupyter notebook file."), i18n("Cantor"));
}


void Worksheet::gotResult(Cantor::Expression* expr)
{
    if(expr==nullptr)
        expr=qobject_cast<Cantor::Expression*>(sender());

    if(expr==nullptr)
        return;

    //We're only interested in help results, others are handled by the WorksheetEntry
    for (auto* result : expr->results())
    {
        if(result && result->type()==Cantor::HelpResult::Type)
        {
            QString help = result->toHtml();
            //Do some basic LaTeX replacing
            help.replace(QRegExp(QLatin1String("\\\\code\\{([^\\}]*)\\}")), QLatin1String("<b>\\1</b>"));
            help.replace(QRegExp(QLatin1String("\\$([^\\$])\\$")), QLatin1String("<i>\\1</i>"));

            emit showHelp(help);

            //TODO: break after the first help result found, not clear yet how to handle multiple requests for help within one single command (e.g. ??ev;??int).
            break;
        }
    }
}

void Worksheet::removeCurrentEntry()
{
    qDebug()<<"removing current entry";
    WorksheetEntry* entry=currentEntry();
    if(!entry)
        return;

    // In case we just removed this
    if (entry->isAncestorOf(m_lastFocusedTextItem))
        m_lastFocusedTextItem = nullptr;
    entry->startRemoving();
}

EpsRenderer* Worksheet::epsRenderer()
{
    return &m_epsRenderer;
}

QMenu* Worksheet::createContextMenu()
{
    QMenu *menu = new QMenu(worksheetView());
    connect(menu, SIGNAL(aboutToHide()), menu, SLOT(deleteLater()));

    return menu;
}

void Worksheet::populateMenu(QMenu *menu, QPointF pos)
{
    WorksheetEntry* entry = entryAt(pos);
    if (entry && !entry->isAncestorOf(m_lastFocusedTextItem)) {
        WorksheetTextItem* item =
            qgraphicsitem_cast<WorksheetTextItem*>(itemAt(pos, QTransform()));
        if (item && item->isEditable())
            m_lastFocusedTextItem = item;
    }

    if (!isRunning())
        menu->addAction(QIcon::fromTheme(QLatin1String("system-run")), i18n("Evaluate Worksheet"),
                        this, SLOT(evaluate()), 0);
    else
        menu->addAction(QIcon::fromTheme(QLatin1String("process-stop")), i18n("Interrupt"), this,
                        SLOT(interrupt()), 0);
    menu->addSeparator();

    if (entry) {
        QMenu* insert = new QMenu(menu);
        QMenu* insertBefore = new QMenu(menu);

        insert->addAction(QIcon::fromTheme(QLatin1String("run-build")), i18n("Command Entry"), entry, SLOT(insertCommandEntry()));
        insert->addAction(QIcon::fromTheme(QLatin1String("draw-text")), i18n("Text Entry"), entry, SLOT(insertTextEntry()));
#ifdef Discount_FOUND
        insert->addAction(QIcon::fromTheme(QLatin1String("text-x-markdown")), i18n("Markdown Entry"), entry, SLOT(insertMarkdownEntry()));
#endif
#ifdef WITH_EPS
        insert->addAction(QIcon::fromTheme(QLatin1String("text-x-tex")), i18n("LaTeX Entry"), entry, SLOT(insertLatexEntry()));
#endif
        insert->addAction(QIcon::fromTheme(QLatin1String("image-x-generic")), i18n("Image"), entry, SLOT(insertImageEntry()));
        insert->addAction(QIcon::fromTheme(QLatin1String("go-next-view-page")), i18n("Page Break"), entry, SLOT(insertPageBreakEntry()));

        insertBefore->addAction(QIcon::fromTheme(QLatin1String("run-build")), i18n("Command Entry"), entry, SLOT(insertCommandEntryBefore()));
        insertBefore->addAction(QIcon::fromTheme(QLatin1String("draw-text")), i18n("Text Entry"), entry, SLOT(insertTextEntryBefore()));
#ifdef Discount_FOUND
        insertBefore->addAction(QIcon::fromTheme(QLatin1String("text-x-markdown")), i18n("Markdown Entry"), entry, SLOT(insertMarkdownEntryBefore()));
#endif
#ifdef WITH_EPS
        insertBefore->addAction(QIcon::fromTheme(QLatin1String("text-x-tex")), i18n("LaTeX Entry"), entry, SLOT(insertLatexEntryBefore()));
#endif
        insertBefore->addAction(QIcon::fromTheme(QLatin1String("image-x-generic")), i18n("Image"), entry, SLOT(insertImageEntryBefore()));
        insertBefore->addAction(QIcon::fromTheme(QLatin1String("go-next-view-page")), i18n("Page Break"), entry, SLOT(insertPageBreakEntryBefore()));

        insert->setTitle(i18n("Insert Entry After"));
        insert->setIcon(QIcon::fromTheme(QLatin1String("edit-table-insert-row-below")));
        insertBefore->setTitle(i18n("Insert Entry Before"));
        insertBefore->setIcon(QIcon::fromTheme(QLatin1String("edit-table-insert-row-above")));
        menu->addMenu(insert);
        menu->addMenu(insertBefore);
    } else {
        menu->addAction(QIcon::fromTheme(QLatin1String("run-build")), i18n("Insert Command Entry"), this, SLOT(appendCommandEntry()));
        menu->addAction(QIcon::fromTheme(QLatin1String("draw-text")), i18n("Insert Text Entry"), this, SLOT(appendTextEntry()));
#ifdef Discount_FOUND
        menu->addAction(QIcon::fromTheme(QLatin1String("text-x-markdown")), i18n("Insert Markdown Entry"), this, SLOT(appendMarkdownEntry()));
#endif
#ifdef WITH_EPS
        menu->addAction(QIcon::fromTheme(QLatin1String("text-x-tex")), i18n("Insert LaTeX Entry"), this, SLOT(appendLatexEntry()));
#endif
        menu->addAction(QIcon::fromTheme(QLatin1String("image-x-generic")), i18n("Insert Image"), this, SLOT(appendImageEntry()));
        menu->addAction(QIcon::fromTheme(QLatin1String("go-next-view-page")), i18n("Insert Page Break"), this, SLOT(appendPageBreakEntry()));
    }
}

void Worksheet::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (m_readOnly)
        return;

    // forward the event to the items
    QGraphicsScene::contextMenuEvent(event);

    if (!event->isAccepted()) {
        event->accept();
        QMenu *menu = createContextMenu();
        populateMenu(menu, event->scenePos());

        menu->popup(event->screenPos());
    }
}

void Worksheet::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mousePressEvent(event);
    /*
    if (event->button() == Qt::LeftButton && !focusItem() && lastEntry() &&
        event->scenePos().y() > lastEntry()->y() + lastEntry()->size().height())
        lastEntry()->focusEntry(WorksheetTextItem::BottomRight);
    */
    if (!m_readOnly)
        updateEntryCursor(event);
}

void Worksheet::keyPressEvent(QKeyEvent *keyEvent)
{
    if (m_readOnly)
        return;

    // If we choose entry by entry cursor and press text button (not modifiers, for example, like Control)
    if ((m_choosenCursorEntry || m_isCursorEntryAfterLastEntry) && !keyEvent->text().isEmpty())
        addEntryFromEntryCursor();

    QGraphicsScene::keyPressEvent(keyEvent);
}

void Worksheet::createActions(KActionCollection* collection)
{
    // Mostly copied from KRichTextWidget::createActions(KActionCollection*)
    // It would be great if this wasn't necessary.

    // Text color
    QAction * action;
    /* This is "format-stroke-color" in KRichTextWidget */
    action = new QAction(QIcon::fromTheme(QLatin1String("format-text-color")),
                         i18nc("@action", "Text &Color..."), collection);
    action->setIconText(i18nc("@label text color", "Color"));
    action->setPriority(QAction::LowPriority);
    m_richTextActionList.append(action);
    collection->addAction(QLatin1String("format_text_foreground_color"), action);
    connect(action, SIGNAL(triggered()), this, SLOT(setTextForegroundColor()));

    // Text color
    action = new QAction(QIcon::fromTheme(QLatin1String("format-fill-color")),
                         i18nc("@action", "Text &Highlight..."), collection);
    action->setPriority(QAction::LowPriority);
    m_richTextActionList.append(action);
    collection->addAction(QLatin1String("format_text_background_color"), action);
    connect(action, SIGNAL(triggered()), this, SLOT(setTextBackgroundColor()));

    // Font Family
    m_fontAction = new KFontAction(i18nc("@action", "&Font"), collection);
    m_richTextActionList.append(m_fontAction);
    collection->addAction(QLatin1String("format_font_family"), m_fontAction);
    connect(m_fontAction, SIGNAL(triggered(QString)), this,
            SLOT(setFontFamily(QString)));

    // Font Size
    m_fontSizeAction = new KFontSizeAction(i18nc("@action", "Font &Size"),
                                           collection);
    m_richTextActionList.append(m_fontSizeAction);
    collection->addAction(QLatin1String("format_font_size"), m_fontSizeAction);
    connect(m_fontSizeAction, SIGNAL(fontSizeChanged(int)), this,
            SLOT(setFontSize(int)));

    // Bold
    m_boldAction = new KToggleAction(QIcon::fromTheme(QLatin1String("format-text-bold")),
                                     i18nc("@action boldify selected text", "&Bold"),
                                     collection);
    m_boldAction->setPriority(QAction::LowPriority);
    QFont bold;
    bold.setBold(true);
    m_boldAction->setFont(bold);
    m_richTextActionList.append(m_boldAction);
    collection->addAction(QLatin1String("format_text_bold"), m_boldAction);
    collection->setDefaultShortcut(m_boldAction, Qt::CTRL + Qt::Key_B);
    connect(m_boldAction, SIGNAL(triggered(bool)), this, SLOT(setTextBold(bool)));

    // Italic
    m_italicAction = new KToggleAction(QIcon::fromTheme(QLatin1String("format-text-italic")),
                                       i18nc("@action italicize selected text",
                                             "&Italic"),
                                       collection);
    m_italicAction->setPriority(QAction::LowPriority);
    QFont italic;
    italic.setItalic(true);
    m_italicAction->setFont(italic);
    m_richTextActionList.append(m_italicAction);
    collection->addAction(QLatin1String("format_text_italic"), m_italicAction);
    collection->setDefaultShortcut(m_italicAction, Qt::CTRL + Qt::Key_I);
    connect(m_italicAction, SIGNAL(triggered(bool)), this, SLOT(setTextItalic(bool)));

    // Underline
    m_underlineAction = new KToggleAction(QIcon::fromTheme(QLatin1String("format-text-underline")),
                                          i18nc("@action underline selected text",
                                                "&Underline"),
                                          collection);
    m_underlineAction->setPriority(QAction::LowPriority);
    QFont underline;
    underline.setUnderline(true);
    m_underlineAction->setFont(underline);
    m_richTextActionList.append(m_underlineAction);
    collection->addAction(QLatin1String("format_text_underline"), m_underlineAction);
    collection->setDefaultShortcut(m_underlineAction, Qt::CTRL + Qt::Key_U);
    connect(m_underlineAction, SIGNAL(triggered(bool)), this, SLOT(setTextUnderline(bool)));

    // Strike
    m_strikeOutAction = new KToggleAction(QIcon::fromTheme(QLatin1String("format-text-strikethrough")),
                                          i18nc("@action", "&Strike Out"),
                                          collection);
    m_strikeOutAction->setPriority(QAction::LowPriority);
    m_richTextActionList.append(m_strikeOutAction);
    collection->addAction(QLatin1String("format_text_strikeout"), m_strikeOutAction);
    collection->setDefaultShortcut(m_strikeOutAction, Qt::CTRL + Qt::Key_L);
    connect(m_strikeOutAction, SIGNAL(triggered(bool)), this, SLOT(setTextStrikeOut(bool)));

    // Alignment
    QActionGroup *alignmentGroup = new QActionGroup(this);

    //   Align left
    m_alignLeftAction = new KToggleAction(QIcon::fromTheme(QLatin1String("format-justify-left")),
                                          i18nc("@action", "Align &Left"),
                                          collection);
    m_alignLeftAction->setPriority(QAction::LowPriority);
    m_alignLeftAction->setIconText(i18nc("@label left justify", "Left"));
    m_richTextActionList.append(m_alignLeftAction);
    collection->addAction(QLatin1String("format_align_left"), m_alignLeftAction);
    connect(m_alignLeftAction, SIGNAL(triggered()), this,
            SLOT(setAlignLeft()));
    alignmentGroup->addAction(m_alignLeftAction);

     //   Align center
    m_alignCenterAction = new KToggleAction(QIcon::fromTheme(QLatin1String("format-justify-center")),
                                            i18nc("@action", "Align &Center"),
                                            collection);
    m_alignCenterAction->setPriority(QAction::LowPriority);
    m_alignCenterAction->setIconText(i18nc("@label center justify", "Center"));
    m_richTextActionList.append(m_alignCenterAction);
    collection->addAction(QLatin1String("format_align_center"), m_alignCenterAction);
    connect(m_alignCenterAction, SIGNAL(triggered()), this,
            SLOT(setAlignCenter()));
    alignmentGroup->addAction(m_alignCenterAction);

    //   Align right
    m_alignRightAction = new KToggleAction(QIcon::fromTheme(QLatin1String("format-justify-right")),
                                           i18nc("@action", "Align &Right"),
                                           collection);
    m_alignRightAction->setPriority(QAction::LowPriority);
    m_alignRightAction->setIconText(i18nc("@label right justify", "Right"));
    m_richTextActionList.append(m_alignRightAction);
    collection->addAction(QLatin1String("format_align_right"), m_alignRightAction);
    connect(m_alignRightAction, SIGNAL(triggered()), this,
            SLOT(setAlignRight()));
    alignmentGroup->addAction(m_alignRightAction);

    //   Align justify
    m_alignJustifyAction = new KToggleAction(QIcon::fromTheme(QLatin1String("format-justify-fill")),
                                             i18nc("@action", "&Justify"),
                                             collection);
    m_alignJustifyAction->setPriority(QAction::LowPriority);
    m_alignJustifyAction->setIconText(i18nc("@label justify fill", "Justify"));
    m_richTextActionList.append(m_alignJustifyAction);
    collection->addAction(QLatin1String("format_align_justify"), m_alignJustifyAction);
    connect(m_alignJustifyAction, SIGNAL(triggered()), this,
            SLOT(setAlignJustify()));
    alignmentGroup->addAction(m_alignJustifyAction);

     /*
     // List style
     KSelectAction* selAction;
     selAction = new KSelectAction(QIcon::fromTheme("format-list-unordered"),
                                   i18nc("@title:menu", "List Style"),
                                   collection);
     QStringList listStyles;
     listStyles      << i18nc("@item:inmenu no list style", "None")
                     << i18nc("@item:inmenu disc list style", "Disc")
                     << i18nc("@item:inmenu circle list style", "Circle")
                     << i18nc("@item:inmenu square list style", "Square")
                     << i18nc("@item:inmenu numbered lists", "123")
                     << i18nc("@item:inmenu lowercase abc lists", "abc")
                     << i18nc("@item:inmenu uppercase abc lists", "ABC");
     selAction->setItems(listStyles);
     selAction->setCurrentItem(0);
     action = selAction;
     m_richTextActionList.append(action);
     collection->addAction("format_list_style", action);
     connect(action, SIGNAL(triggered(int)),
             this, SLOT(_k_setListStyle(int)));
     connect(action, SIGNAL(triggered()),
             this, SLOT(_k_updateMiscActions()));

     // Indent
     action = new QAction(QIcon::fromTheme("format-indent-more"),
                          i18nc("@action", "Increase Indent"), collection);
     action->setPriority(QAction::LowPriority);
     m_richTextActionList.append(action);
     collection->addAction("format_list_indent_more", action);
     connect(action, SIGNAL(triggered()),
             this, SLOT(indentListMore()));
     connect(action, SIGNAL(triggered()),
             this, SLOT(_k_updateMiscActions()));

     // Dedent
     action = new QAction(QIcon::fromTheme("format-indent-less"),
                          i18nc("@action", "Decrease Indent"), collection);
     action->setPriority(QAction::LowPriority);
     m_richTextActionList.append(action);
     collection->addAction("format_list_indent_less", action);
     connect(action, SIGNAL(triggered()), this, SLOT(indentListLess()));
     connect(action, SIGNAL(triggered()), this, SLOT(_k_updateMiscActions()));
     */
}

WorksheetTextItem* Worksheet::lastFocusedTextItem()
{
    return m_lastFocusedTextItem;
}

void Worksheet::updateFocusedTextItem(WorksheetTextItem* newItem)
{
    // No need update and emit signals about editing actions in readonly
    // So support only copy action and reset selection
    if (m_readOnly)
    {
        if (m_lastFocusedTextItem && m_lastFocusedTextItem != newItem)
        {
            disconnect(this, SIGNAL(copy()), m_lastFocusedTextItem, SLOT(copy()));
            m_lastFocusedTextItem->clearSelection();
        }

        if (newItem && m_lastFocusedTextItem != newItem)
        {
            connect(this, SIGNAL(copy()), newItem, SLOT(copy()));
            emit copyAvailable(newItem->isCopyAvailable());
        }
        else if (!newItem)
        {
            emit copyAvailable(false);
        }

        m_lastFocusedTextItem = newItem;
        return;
    }

    if (m_lastFocusedTextItem && m_lastFocusedTextItem != newItem) {
        disconnect(m_lastFocusedTextItem, SIGNAL(undoAvailable(bool)),
                   this, SIGNAL(undoAvailable(bool)));
        disconnect(m_lastFocusedTextItem, SIGNAL(redoAvailable(bool)),
                   this, SIGNAL(redoAvailable(bool)));
        disconnect(this, SIGNAL(undo()), m_lastFocusedTextItem, SLOT(undo()));
        disconnect(this, SIGNAL(redo()), m_lastFocusedTextItem, SLOT(redo()));
        disconnect(m_lastFocusedTextItem, SIGNAL(cutAvailable(bool)),
                   this, SIGNAL(cutAvailable(bool)));
        disconnect(m_lastFocusedTextItem, SIGNAL(copyAvailable(bool)),
                   this, SIGNAL(copyAvailable(bool)));
        disconnect(m_lastFocusedTextItem, SIGNAL(pasteAvailable(bool)),
                   this, SIGNAL(pasteAvailable(bool)));
        disconnect(this, SIGNAL(cut()), m_lastFocusedTextItem, SLOT(cut()));
        disconnect(this, SIGNAL(copy()), m_lastFocusedTextItem, SLOT(copy()));

        m_lastFocusedTextItem->clearSelection();
    }

    if (newItem && m_lastFocusedTextItem != newItem) {
        setAcceptRichText(newItem->richTextEnabled());
        emit undoAvailable(newItem->isUndoAvailable());
        emit redoAvailable(newItem->isRedoAvailable());
        connect(newItem, SIGNAL(undoAvailable(bool)),
                this, SIGNAL(undoAvailable(bool)));
        connect(newItem, SIGNAL(redoAvailable(bool)),
                this, SIGNAL(redoAvailable(bool)));
        connect(this, SIGNAL(undo()), newItem, SLOT(undo()));
        connect(this, SIGNAL(redo()), newItem, SLOT(redo()));
        emit cutAvailable(newItem->isCutAvailable());
        emit copyAvailable(newItem->isCopyAvailable());
        emit pasteAvailable(newItem->isPasteAvailable());
        connect(newItem, SIGNAL(cutAvailable(bool)),
                this, SIGNAL(cutAvailable(bool)));
        connect(newItem, SIGNAL(copyAvailable(bool)),
                this, SIGNAL(copyAvailable(bool)));
        connect(newItem, SIGNAL(pasteAvailable(bool)),
                this, SIGNAL(pasteAvailable(bool)));
        connect(this, SIGNAL(cut()), newItem, SLOT(cut()));
        connect(this, SIGNAL(copy()), newItem, SLOT(copy()));
    } else if (!newItem) {
        emit undoAvailable(false);
        emit redoAvailable(false);
        emit cutAvailable(false);
        emit copyAvailable(false);
        emit pasteAvailable(false);
    }
    m_lastFocusedTextItem = newItem;
}

/*!
 * handles the paste action triggered in cantor_part.
 * Pastes into the last focused text item.
 * In case the "new entry"-cursor is currently shown,
 * a new entry is created first which the content will be pasted into.
 */
void Worksheet::paste() {
    if (m_choosenCursorEntry || m_isCursorEntryAfterLastEntry)
        addEntryFromEntryCursor();

    m_lastFocusedTextItem->paste();
}

void Worksheet::setRichTextInformation(const RichTextInfo& info)
{
    m_boldAction->setChecked(info.bold);
    m_italicAction->setChecked(info.italic);
    m_underlineAction->setChecked(info.underline);
    m_strikeOutAction->setChecked(info.strikeOut);
    m_fontAction->setFont(info.font);
    if (info.fontSize > 0)
        m_fontSizeAction->setFontSize(info.fontSize);

    if (info.align & Qt::AlignLeft)
        m_alignLeftAction->setChecked(true);
    else if (info.align & Qt::AlignCenter)
        m_alignCenterAction->setChecked(true);
    else if (info.align & Qt::AlignRight)
        m_alignRightAction->setChecked(true);
    else if (info.align & Qt::AlignJustify)
        m_alignJustifyAction->setChecked(true);
}

void Worksheet::setAcceptRichText(bool b)
{
    if (!m_readOnly)
        for(QAction * action : m_richTextActionList)
            action->setEnabled(b);
}

WorksheetTextItem* Worksheet::currentTextItem()
{
    QGraphicsItem* item = focusItem();
    if (!item)
        item = m_lastFocusedTextItem;
    while (item && item->type() != WorksheetTextItem::Type)
        item = item->parentItem();

    return qgraphicsitem_cast<WorksheetTextItem*>(item);
}

void Worksheet::setTextForegroundColor()
{
    WorksheetTextItem* item = currentTextItem();
    if (item)
        item->setTextForegroundColor();
}

void Worksheet::setTextBackgroundColor()
{
    WorksheetTextItem* item = currentTextItem();
    if (item)
        item->setTextBackgroundColor();
}

void Worksheet::setTextBold(bool b)
{
    WorksheetTextItem* item = currentTextItem();
    if (item)
        item->setTextBold(b);
}

void Worksheet::setTextItalic(bool b)
{
    WorksheetTextItem* item = currentTextItem();
    if (item)
        item->setTextItalic(b);
}

void Worksheet::setTextUnderline(bool b)
{
    WorksheetTextItem* item = currentTextItem();
    if (item)
        item->setTextUnderline(b);
}

void Worksheet::setTextStrikeOut(bool b)
{
    WorksheetTextItem* item = currentTextItem();
    if (item)
        item->setTextStrikeOut(b);
}

void Worksheet::setAlignLeft()
{
    WorksheetTextItem* item = currentTextItem();
    if (item)
        item->setAlignment(Qt::AlignLeft);
}

void Worksheet::setAlignRight()
{
    WorksheetTextItem* item = currentTextItem();
    if (item)
        item->setAlignment(Qt::AlignRight);
}

void Worksheet::setAlignCenter()
{
    WorksheetTextItem* item = currentTextItem();
    if (item)
        item->setAlignment(Qt::AlignCenter);
}

void Worksheet::setAlignJustify()
{
    WorksheetTextItem* item = currentTextItem();
    if (item)
        item->setAlignment(Qt::AlignJustify);
}

void Worksheet::setFontFamily(const QString& font)
{
    WorksheetTextItem* item = currentTextItem();
    if (item)
        item->setFontFamily(font);
}

void Worksheet::setFontSize(int size)
{
    WorksheetTextItem* item = currentTextItem();
    if (item)
        item->setFontSize(size);
}

bool Worksheet::isShortcut(const QKeySequence& sequence)
{
    return m_shortcuts.contains(sequence);
}

void Worksheet::registerShortcut(QAction* action)
{
    for (auto& shortcut : action->shortcuts())
        m_shortcuts.insert(shortcut, action);

    connect(action, SIGNAL(changed()), this, SLOT(updateShortcut()));
}

void Worksheet::updateShortcut()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action)
        return;

    // delete the old shortcuts of this action
    QList<QKeySequence> shortcuts = m_shortcuts.keys(action);
    for (auto& shortcut : shortcuts)
        m_shortcuts.remove(shortcut);

    // add the new shortcuts
    for (auto& shortcut : action->shortcuts())
        m_shortcuts.insert(shortcut, action);
}

void Worksheet::dragEnterEvent(QGraphicsSceneDragDropEvent* event)
{
    qDebug() << "enter";
    if (m_dragEntry)
        event->accept();
    else
        QGraphicsScene::dragEnterEvent(event);
}

void Worksheet::dragLeaveEvent(QGraphicsSceneDragDropEvent* event)
{
    if (!m_dragEntry) {
        QGraphicsScene::dragLeaveEvent(event);
        return;
    }

    qDebug() << "leave";
    event->accept();
    if (m_placeholderEntry) {
        m_placeholderEntry->startRemoving();
        m_placeholderEntry = nullptr;
    }
}

void Worksheet::dragMoveEvent(QGraphicsSceneDragDropEvent* event)
{
    if (!m_dragEntry) {
        QGraphicsScene::dragMoveEvent(event);
        return;
    }

    QPointF pos = event->scenePos();
    WorksheetEntry* entry = entryAt(pos);
    WorksheetEntry* prev = nullptr;
    WorksheetEntry* next = nullptr;
    if (entry) {
        if (pos.y() < entry->y() + entry->size().height()/2) {
            prev = entry->previous();
            next = entry;
        } else if (pos.y() >= entry->y() + entry->size().height()/2) {
            prev = entry;
            next = entry->next();
        }
    } else {
        WorksheetEntry* last = lastEntry();
        if (last && pos.y() > last->y() + last->size().height()) {
            prev = last;
            next = nullptr;
        }
    }

    if (prev || next) {
        PlaceHolderEntry* oldPlaceHolder = m_placeholderEntry;
        if (prev && prev->type() == PlaceHolderEntry::Type &&
            (!prev->aboutToBeRemoved() || prev->stopRemoving())) {
            m_placeholderEntry = qgraphicsitem_cast<PlaceHolderEntry*>(prev);
            m_placeholderEntry->changeSize(m_dragEntry->size());
        } else if (next && next->type() == PlaceHolderEntry::Type &&
                   (!next->aboutToBeRemoved() || next->stopRemoving())) {
            m_placeholderEntry = qgraphicsitem_cast<PlaceHolderEntry*>(next);
            m_placeholderEntry->changeSize(m_dragEntry->size());
        } else {
            m_placeholderEntry = new PlaceHolderEntry(this, QSizeF(0,0));
            m_placeholderEntry->setPrevious(prev);
            m_placeholderEntry->setNext(next);
            if (prev)
                prev->setNext(m_placeholderEntry);
            else
                setFirstEntry(m_placeholderEntry);
            if (next)
                next->setPrevious(m_placeholderEntry);
            else
                setLastEntry(m_placeholderEntry);
            m_placeholderEntry->changeSize(m_dragEntry->size());
        }
        if (oldPlaceHolder && oldPlaceHolder != m_placeholderEntry)
            oldPlaceHolder->startRemoving();
        updateLayout();
    }

    const QPoint viewPos = worksheetView()->mapFromScene(pos);
    const int viewHeight = worksheetView()->viewport()->height();
    if ((viewPos.y() < 10 || viewPos.y() > viewHeight - 10) &&
        !m_dragScrollTimer) {
        m_dragScrollTimer = new QTimer(this);
        m_dragScrollTimer->setSingleShot(true);
        m_dragScrollTimer->setInterval(100);
        connect(m_dragScrollTimer, SIGNAL(timeout()), this,
                SLOT(updateDragScrollTimer()));
        m_dragScrollTimer->start();
    }

    event->accept();
}

void Worksheet::dropEvent(QGraphicsSceneDragDropEvent* event)
{
    if (!m_dragEntry)
        QGraphicsScene::dropEvent(event);
    event->accept();
}

void Worksheet::updateDragScrollTimer()
{
    if (!m_dragScrollTimer)
        return;

    const QPoint viewPos = worksheetView()->viewCursorPos();
    const QWidget* viewport = worksheetView()->viewport();
    const int viewHeight = viewport->height();
    if (!m_dragEntry || !(viewport->rect().contains(viewPos)) ||
        (viewPos.y() >= 10 && viewPos.y() <= viewHeight - 10)) {
        delete m_dragScrollTimer;
        m_dragScrollTimer = nullptr;
        return;
    }

    if (viewPos.y() < 10)
        worksheetView()->scrollBy(-10*(10 - viewPos.y()));
    else
        worksheetView()->scrollBy(10*(viewHeight - viewPos.y()));

    m_dragScrollTimer->start();
}

void Worksheet::updateEntryCursor(QGraphicsSceneMouseEvent* event)
{
    // determine the worksheet entry near which the entry cursor will be shown
    resetEntryCursor();
    if (event->button() == Qt::LeftButton && !focusItem())
    {
        const qreal y = event->scenePos().y();
        for (WorksheetEntry* entry = firstEntry(); entry; entry = entry->next())
        {
            if (entry == firstEntry() && y < entry->y() )
            {
                m_choosenCursorEntry = firstEntry();
                break;
            }
            else if (entry->y() < y && (entry->next() && y < entry->next()->y()))
            {
                m_choosenCursorEntry = entry->next();
                break;
            }
            else if (entry->y() < y && entry == lastEntry())
            {
                m_isCursorEntryAfterLastEntry = true;
                break;
            }
        }
    }

    if (m_choosenCursorEntry || m_isCursorEntryAfterLastEntry)
        drawEntryCursor();
}

void Worksheet::addEntryFromEntryCursor()
{
    qDebug() << "Add new entry from entry cursor";
    if (m_isCursorEntryAfterLastEntry)
        insertCommandEntry(lastEntry());
    else
        insertCommandEntryBefore(m_choosenCursorEntry);
    resetEntryCursor();
}

void Worksheet::animateEntryCursor()
{
    if ((m_choosenCursorEntry || m_isCursorEntryAfterLastEntry) && m_entryCursorItem)
        m_entryCursorItem->setVisible(!m_entryCursorItem->isVisible());
}

void Worksheet::resetEntryCursor()
{
    m_choosenCursorEntry = nullptr;
    m_isCursorEntryAfterLastEntry = false;
    m_entryCursorItem->hide();
}

void Worksheet::drawEntryCursor()
{
    if (m_entryCursorItem && (m_choosenCursorEntry || (m_isCursorEntryAfterLastEntry && lastEntry())))
    {
        qreal x;
        qreal y;
        if (m_isCursorEntryAfterLastEntry)
        {
            x = lastEntry()->x();
            y = lastEntry()->y() + lastEntry()->size().height() - (EntryCursorWidth - 1);
        }
        else
        {
            x = m_choosenCursorEntry->x();
            y = m_choosenCursorEntry->y();
        }
        m_entryCursorItem->setLine(x,y,x+EntryCursorLength,y);
        m_entryCursorItem->show();
    }
}

void Worksheet::setType(Worksheet::Type type)
{
    m_type = type;
}

Worksheet::Type Worksheet::type() const
{
    return m_type;
}
