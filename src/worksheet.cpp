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

#include <QGraphicsWidget>
#include <QTextLayout>
#include <QTextDocument>
#include <QTimer>
#include <QPrinter>
#include <QXmlQuery>

#include <KMessageBox>
#include <KStandardDirs>

#include "config-cantor.h"
#include "worksheet.h"
#include "settings.h"
#include "commandentry.h"
#include "textentry.h"
#include "latexentry.h"
#include "imageentry.h"
#include "pagebreakentry.h"
#include "lib/backend.h"
#include "lib/extension.h"
#include "lib/result.h"
#include "lib/helpresult.h"
#include "lib/session.h"
#include "lib/defaulthighlighter.h"

const double Worksheet::LeftMargin = 4;
const double Worksheet::RightMargin = 4;
const double Worksheet::TopMargin = 4;

Worksheet::Worksheet(Cantor::Backend* backend, QWidget* parent)
    : QGraphicsScene(parent)
{
    m_session = backend->createSession();
    m_highlighter = 0;

    m_firstEntry = 0;
    m_lastEntry = 0;
    m_currentEntry = 0;
    m_dragEntry = 0;
    m_width = 0;

    m_isPrinting = false;
    m_loginFlag = true;
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
	enableAnimations(Settings::self()->animationDefault());
#ifdef WITH_EPS
        session()->setTypesettingEnabled(Settings::self()->typesetDefault());
#else
        session()->setTypesettingEnabled(false);
#endif

        m_loginFlag=false;
    }
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
    qreal y = 0;

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

    m_width = w;
    if (s != m_epsRenderer.scale() || forceUpdate) {
	m_epsRenderer.setScale(s);
	for (WorksheetEntry *entry = firstEntry(); entry; entry = entry->next())
	    entry->updateEntry();
    }
    updateLayout();
}

void Worksheet::updateLayout()
{
    const qreal w = m_width - LeftMargin - RightMargin;
    qreal y = TopMargin;
    const qreal x = LeftMargin;
    for (WorksheetEntry *entry = firstEntry(); entry; entry = entry->next())
	y += entry->setGeometry(x, y, w);
    setSceneRect(QRectF(0, 0, m_width, y));
}

void Worksheet::updateEntrySize(WorksheetEntry* entry)
{
    qreal y = entry->y() + entry->size().height();
    for (entry = entry->next(); entry; entry = entry->next()) {
	entry->setY(y);
	y += entry->size().height();
    }
    setSceneRect(QRectF(0, 0, m_width, y));
}

qreal Worksheet::contentsWidth()
{
    return m_width - LeftMargin - RightMargin;
}

bool Worksheet::isEmpty()
{
    return !m_firstEntry;
}

WorksheetView* Worksheet::worksheetView()
{
    return qobject_cast<WorksheetView*>(views()[0]);
}

void Worksheet::setModified()
{
    emit modified();
}

WorksheetEntry* Worksheet::currentEntry()
{
    QGraphicsItem* item = focusItem();
    while (item && (item->type() < QGraphicsItem::UserType ||
		    item->type() >= QGraphicsItem::UserType + 100))
	item = item->parentItem();
    if (item) {
	m_currentEntry = qobject_cast<WorksheetEntry*>(item->toGraphicsObject());
	return m_currentEntry;
    } else {
	return m_currentEntry;
    }
    return 0;
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
    m_firstEntry = entry;
}

void Worksheet::setLastEntry(WorksheetEntry* entry)
{
    m_lastEntry = entry;
}

WorksheetEntry* Worksheet::entryAt(qreal x, qreal y)
{
    QGraphicsItem* item = itemAt(x, y);
    while (item && (item->type() < QGraphicsItem::UserType ||
		    item->type() >= QGraphicsItem::UserType + 100))
	item = item->parentItem();
    if (item)
	return qobject_cast<WorksheetEntry*>(item->toGraphicsObject());
    return 0;
}

void Worksheet::focusEntry(WorksheetEntry *entry)
{
    if (!entry)
        return;
    entry->focusEntry();
    //bool rt = entry->acceptRichText();
    //setActionsEnabled(rt);
    //setAcceptRichText(rt);
    //ensureCursorVisible();
}

void Worksheet::startDrag(WorksheetEntry* entry, QDrag* drag)
{
    entry->setOpacity(0);
    m_dragEntry = entry;
    drag->exec();
}

void Worksheet::evaluate()
{
    kDebug()<<"evaluate worksheet";
    firstEntry()->evaluate(WorksheetEntry::EvaluateNextEntries);

    emit modified();
}

void Worksheet::evaluateCurrentEntry()
{
    kDebug() << "evaluation requested...";
    WorksheetEntry* entry = currentEntry();
    if(!entry)
        return;
    entry->evaluate(WorksheetEntry::FocusedItemOnly);
}

bool Worksheet::completionEnabled()
{
    return m_completionEnabled;
}

void Worksheet::showCompletion()
{
    WorksheetEntry* current = currentEntry();
    current->showCompletion();
}

WorksheetEntry* Worksheet::appendEntry(const int type)
{
    WorksheetEntry* entry = WorksheetEntry::create(type, this);

    if (entry)
    {
        kDebug() << "Entry Appended";
	addItem(entry);
	entry->setPrevious(lastEntry());
	if (lastEntry())
	    lastEntry()->setNext(entry);
	if (!firstEntry())
	    m_firstEntry = entry;
	m_lastEntry = entry;
	updateLayout();
        focusEntry(entry);
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


WorksheetEntry* Worksheet::insertEntry(const int type)
{
    WorksheetEntry *current = currentEntry();

    if (!current)
	return 0;

    WorksheetEntry *next = current->next();
    WorksheetEntry *entry = 0;

    if (!next || next->type() != type || !next->isEmpty())
    {
	entry = WorksheetEntry::create(type, this);
	addItem(entry);
	entry->setPrevious(current);
	entry->setNext(next);
	current->setNext(entry);
	if (next)
	    next->setPrevious(entry);
	else
	    m_lastEntry = entry;
	updateLayout();
    }

    focusEntry(entry);
    return entry;
}

WorksheetEntry* Worksheet::insertTextEntry()
{
    return insertEntry(TextEntry::Type);
}

WorksheetEntry* Worksheet::insertCommandEntry()
{
    return insertEntry(CommandEntry::Type);
}

WorksheetEntry* Worksheet::insertImageEntry()
{
    return insertEntry(ImageEntry::Type);
}

WorksheetEntry* Worksheet::insertPageBreakEntry()
{
    return insertEntry(PageBreakEntry::Type);
}

WorksheetEntry* Worksheet::insertLatexEntry()
{
    return insertEntry(LatexEntry::Type);
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
    WorksheetEntry *current = currentEntry();

    if (!current)
	return 0;

    WorksheetEntry *prev = current->previous();
    WorksheetEntry *entry = 0;

    if(!prev || prev->type() != type || !prev->isEmpty())
    {
	entry = WorksheetEntry::create(type, this);
	addItem(entry);
	entry->setNext(current);
	entry->setPrevious(prev);
	current->setPrevious(entry);
	if (prev)
	    prev->setNext(entry);
	else
	    m_firstEntry = entry;
	updateLayout();
    }

    focusEntry(entry);
    return entry;
}

WorksheetEntry* Worksheet::insertTextEntryBefore()
{
    return insertEntryBefore(TextEntry::Type);
}

WorksheetEntry* Worksheet::insertCommandEntryBefore()
{
    return insertEntryBefore(CommandEntry::Type);
}

WorksheetEntry* Worksheet::insertPageBreakEntryBefore()
{
    return insertEntryBefore(PageBreakEntry::Type);
}

WorksheetEntry* Worksheet::insertImageEntryBefore()
{
    return insertEntryBefore(ImageEntry::Type);
}

WorksheetEntry* Worksheet::insertLatexEntryBefore()
{
    return insertEntryBefore(LatexEntry::Type);
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

void Worksheet::highlightItem(WorksheetTextItem* item)
{
    if (!m_highlighter)
	return;

    QTextDocument *oldDocument = m_highlighter->document();
    QList<QList<QTextLayout::FormatRange> > formats;

    if (oldDocument)
	for (QTextBlock b = oldDocument->firstBlock();
	     b.isValid(); b = b.next())
	    formats.append(b.layout()->additionalFormats());

    // Not every highlighter is a Cantor::DefaultHighligther (e.g. the
    // highlighter for KAlgebra)
    Cantor::DefaultHighlighter* hl = qobject_cast<Cantor::DefaultHighlighter*>(m_highlighter);
    if (hl) {
	hl->setTextItem(item);
    } else {
	m_highlighter->setDocument(item->document());
    }

    if (oldDocument)
	for (QTextBlock b = oldDocument->firstBlock();
	     b.isValid(); b = b.next()) {

	    b.layout()->setAdditionalFormats(formats.first());
	    formats.pop_front();
	}

}

void Worksheet::enableHighlighting(bool highlight)
{
    if(highlight) {
        if(m_highlighter)
            m_highlighter->deleteLater();
        m_highlighter=session()->syntaxHighlighter(this);
        if(!m_highlighter)
            m_highlighter=new Cantor::DefaultHighlighter(this);

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
	WorksheetTextItem* textitem = entry ? entry->highlightItem() : 0;
	if (textitem && textitem->hasFocus())
	    highlightItem(textitem);
    } else {
        if(m_highlighter)
            m_highlighter->deleteLater();
        m_highlighter=0;

	// remove highlighting from entries
	WorksheetEntry* entry;
	for (entry = firstEntry(); entry; entry = entry->next()) {
	    WorksheetTextItem* item = entry->highlightItem();
	    if (!item)
		continue;
	    for (QTextBlock b = item->document()->firstBlock();
		 b.isValid(); b = b.next())
		b.layout()->clearAdditionalFormats();
	}
	update();
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

void Worksheet::enableExpressionNumbering(bool enable)
{
    m_showExpressionIds=enable;
    emit updatePrompt();
}

QDomDocument Worksheet::toXML(KZip* archive)
{
    QDomDocument doc( "CantorWorksheet" );
    QDomElement root=doc.createElement( "Worksheet" );
    root.setAttribute("backend", m_session->backend()->name());
    doc.appendChild(root);

    for( WorksheetEntry* entry = firstEntry(); entry; entry = entry->next())
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
        KMessageBox::error( worksheetView(),
			    i18n( "Cannot write file %1." , filename ),
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
        KMessageBox::error(worksheetView(), i18n("Error saving file %1", filename), i18n("Error - Cantor"));
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

    for(WorksheetEntry * entry = firstEntry(); entry; entry = entry->next())
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
        KMessageBox::error(worksheetView(), i18n("Error saving file %1", filename), i18n("Error - Cantor"));
        return;
    }

    QTextStream stream(&file);
    QXmlQuery query(QXmlQuery::XSLT20);
    kDebug() << toXML().toString();
    query.setFocus(toXML().toString());

    QString stylesheet = KStandardDirs::locate("appdata", "xslt/latex.xsl");
    if (stylesheet.isEmpty())
    {
        KMessageBox::error(worksheetView(), i18n("Error loading latex.xsl stylesheet"), i18n("Error - Cantor"));
        return;
    }

    query.setQuery(QUrl(stylesheet));
    QString out;
    if (query.evaluateTo(&out))
        stream << out;
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
        KMessageBox::error(worksheetView(), i18n("The backend with which this file was generated is not installed. It needs %1", backendName), i18n("Cantor"));
        return;
    }

    if(!b->isEnabled())
    {
        KMessageBox::information(worksheetView(), i18n("There are some problems with the %1 backend,\n"\
                                            "please check your configuration or install the needed packages.\n"
                                            "You will only be able to view this worksheet.", backendName), i18n("Cantor"));

    }


    //cleanup the worksheet and all it contains
    delete m_session;
    m_session=0;
    for(WorksheetEntry* entry = firstEntry(); entry; entry = entry->next())
        delete entry;
    clear();
    m_firstEntry = 0;
    m_lastEntry = 0;

    m_session=b->createSession();
    m_loginFlag=true;

    kDebug()<<"loading entries";
    QDomElement expressionChild = root.firstChildElement();
    WorksheetEntry* entry;
    while (!expressionChild.isNull()) {
        QString tag = expressionChild.tagName();
        if (tag == "Expression")
        {
            entry = appendCommandEntry();
            entry->setContent(expressionChild, file);
        } else if (tag == "Text")
        {
            entry = appendTextEntry();
            entry->setContent(expressionChild, file);
        } else if (tag == "Latex")
        {
            entry = appendLatexEntry();
            entry->setContent(expressionChild, file);
        } else if (tag == "PageBreak")
	{
	    entry = appendPageBreakEntry();
	    entry->setContent(expressionChild, file);
	}
	else if (tag == "Image")
	{
	  entry = appendImageEntry();
	  entry->setContent(expressionChild, file);
	}

        expressionChild = expressionChild.nextSiblingElement();
    }

    //login to the session, but let Qt process all the events in its pipeline
    //first.
    QTimer::singleShot(0, this, SLOT(loginToSession()));

    //Set the Highlighting, depending on the current state
    //If the session isn't logged in, use the default
    enableHighlighting( m_highlighter!=0 || (m_loginFlag && Settings::highlightDefault()) );



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
        //Do some basic LaTeX replacing
        help.replace(QRegExp("\\\\code\\{([^\\}]*)\\}"), "<b>\\1</b>");
        help.replace(QRegExp("\\$([^\\$])\\$"), "<i>\\1</i>");

        emit showHelp(help);
    }
}

void Worksheet::removeCurrentEntry()
{
    kDebug()<<"removing current entry";
    WorksheetEntry* entry=currentEntry();
    if(!entry)
        return;

    m_currentEntry = 0;
    entry->startRemoving();
}

EpsRenderer* Worksheet::epsRenderer()
{
    return &m_epsRenderer;
}

KMenu* Worksheet::createContextMenu()
{
    KMenu *menu = new KMenu(worksheetView());
    connect(menu, SIGNAL(aboutToHide()), menu, SLOT(deleteLater()));

    return menu;
}

void Worksheet::populateMenu(KMenu *menu, const QPointF& pos)
{
    Q_UNUSED(pos);

    WorksheetEntry* entry = entryAt(pos.x(), pos.y());
    m_currentEntry = entry;

    if (!isRunning())
	menu->addAction(KIcon("system-run"), i18n("Evaluate Worksheet"),
			this, SLOT(evaluate()), 0);
    else
	menu->addAction(KIcon("process-stop"), i18n("Interrupt"), this,
			SLOT(interrupt()), 0);
    menu->addSeparator();

    if (entry) {
	KMenu* insert = new KMenu(menu);
	KMenu* insertBefore = new KMenu(menu);

	insert->addAction(i18n("Command Entry"), this, SLOT(insertCommandEntry()));
	insert->addAction(i18n("Text Entry"), this, SLOT(insertTextEntry()));
	insert->addAction(i18n("LaTeX Entry"), this, SLOT(insertLatexEntry()));
	insert->addAction(i18n("Image"), this, SLOT(insertImageEntry()));
	insert->addAction(i18n("Page Break"), this, SLOT(insertPageBreakEntry()));

	insertBefore->addAction(i18n("Command Entry"), this, SLOT(insertCommandEntryBefore()));
	insertBefore->addAction(i18n("Text Entry"), this, SLOT(insertTextEntryBefore()));
	insertBefore->addAction(i18n("LaTeX Entry"), this, SLOT(insertLatexEntryBefore()));
	insertBefore->addAction(i18n("Image"), this, SLOT(insertImageEntryBefore()));
	insertBefore->addAction(i18n("Page Break"), this, SLOT(insertPageBreakEntryBefore()));

	insert->setTitle(i18n("Insert"));
	insertBefore->setTitle(i18n("Insert Before"));
	menu->addMenu(insert);
	menu->addMenu(insertBefore);
    } else {
	menu->addAction(i18n("Insert Command Entry"), this, SLOT(appendCommandEntry()));
	menu->addAction(i18n("Insert Text Entry"), this, SLOT(appendTextEntry()));
	menu->addAction(i18n("Insert LaTeX Entry"), this, SLOT(appendLatexEntry()));
	menu->addAction(i18n("Insert Image"), this, SLOT(appendImageEntry()));
	menu->addAction(i18n("Insert Page Break"), this, SLOT(appendPageBreakEntry()));
    }
}

void Worksheet::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    // forward the event to the items
    QGraphicsScene::contextMenuEvent(event);

    if (!event->isAccepted()) {
	event->accept();
	KMenu *menu = createContextMenu();
	populateMenu(menu, event->scenePos());

	menu->popup(event->screenPos());
    }
}

void Worksheet::focusOutEvent(QFocusEvent* focusEvent)
{
    currentEntry();
    QGraphicsScene::focusOutEvent(focusEvent);
}

/*
void Worksheet::dragEnterEvent(QGraphicsSceneDragDropEvent* event)
{
}

void Worksheet::dragLeaveEvent(QGraphicsSceneDragDropEvent* event)
{
}

void Worksheet::dragMoveEvent(QGraphicsSceneDragDropEvent* event)
{
}

void Worksheet::dropEvent(QGraphicsSceneDragDropEvent* event)
{
}
*/

#include "worksheet.moc"
