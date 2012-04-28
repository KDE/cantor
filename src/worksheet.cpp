
#include "worksheet.h"


Worksheet::Worksheet(Cantor::Backend* backend, QWidget* parent) 
    : QGraphicsScene(parent)
{
    m_session = backend->createSession();
    m_rootlayout = new QGraphicsLinearLayout(Qt::Vertical);
    m_rootwidget = new QGraohicsWidget;
    m_rootwidget.setLayout(m_rootlayout);

    // todo: set scene rect

    //...

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

	// ...


        m_loginFlag=false;
    }
}

bool Worksheet::isPrinting()
{
    return m_isPrinting;
}

void Worksheet::setViewSize(qreal w, qreal h)
{
    m_rootwidget.setMaximumWidth(w);
    m_rootwidget.setMinimumHeight(h);
}

WorksheetEntry* Worksheet::currentEntry()
{
    QGraphicsItem* item = focusItem();
    while (item && item->type() < QGraphicsItem::UserType)
	item = item->parentItem();
    if (item)
	return qt_cast<WorksheetEntry*>(item);
    return 0;
}

WorksheetEntry* Worksheet::entryAt(qreal x, qreal y)
{
    QGraphicsItem* item = itemAt(x, y);
    while (item && item->type() < QGraphicsItem::UserType)
	item = item->parentItem();
    if (item)
	return qt_cast<WorksheetEntry*>(item);
    return 0;
}

WorksheetEntry* Worksheet::entryAt(int row)
{
    if (row >= 0 && row < entryCount())
	return m_entries[row];
    return 0;
}

int Worksheet::entryCount()
{
    return m_entries->size();
}

void Worksheet::focusEntry(WorksheetEntry *entry)
{
    if (!entry)
        return;
    // todo: pass information about the cursor position
    entry->setFocus();
    bool rt = entry->acceptRichText();
    //setActionsEnabled(rt);
    //setAcceptRichText(rt);
    //ensureCursorVisible();
}

void Worksheet::moveToPreviousEntry()
{
    int index = m_entries.indexOf(currentEntry());
    kDebug() << "index: " << index;
    --index;
    while (index >= 0 && (m_entries[index].flags() & QGraphicsItem::ItemIsFocusable))
	--index;
    if(index >= 0)
        setCurrentEntry(m_entries[index]);
}

void Worksheet::moveToNextEntry()
{
    int index = m_entries.indexOf(currentEntry());
    kDebug() << "index: " << index;
    ++index;
    while (index < entryCount() && (m_entries[index].flags() & QGraphicsItem::ItemIsFocusable))
	++index;
    if(index < entryCount())
        setCurrentEntry(m_entries[index]);
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

WorksheetEntry* Worksheet::appendEntry(const int type)
{
    WorksheetEntry* entry = WorksheetEntry::create(type);
    m_rootlayout.addItem(entry);
    prev->setOwnedByLayout(false);
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


WorksheetEntry* Worksheet::insertEntry(const int type)
{
    WorksheetEntry *current = currentEntry();
    
    if (!current)
	return 0;
    
    int index = m_entries.indexOf(current);
    WorksheetEntry *next = entryAt(index + 1);

    if (!next || next->type() != type || !next->isEmpty())
    {
	next = WorksheetEntry::create(type);
	m_rootlayout.insertItem(index + 1, next);
	prev->setOwnedByLayout(false);
	m_entries.insert(index+1, next);
    }

    setCurrentEntry(next);
    return next;
}

WorksheetEntry* Worksheet::insertTextEntry()
{
    return insertEntry(TextEntry::Type);
}

WorksheetEntry* Worksheet::insertImageEntry()
{
    return insertEntry(ImageEntry::Type);
}

WorksheetEntry* Worksheet::insertCommandEntry()
{
    return insertEntry(CommandEntry::Type);
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
    
    int index = m_entries.indexOf(current);
    WorksheetEntry *prev = entryAt(index - 1);

    if(!prevE || prevE->type() != type || !prevE->isEmpty())
    {
	prev = WorksheetEntry::create(type);
	m_rootlayout.insertItem(index, prev);
	prev->setOwnedByLayout(false);
	m_entries.insert(index, prev);
    }

    setCurrentEntry(prev);
    return prev;
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

void Worksheet::enableHighlighting(bool highlight)
{
    foreach( WorksheetEntry* entry, m_entries )
    {
	entry->enableHighlighting(highlight);
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


    //cleanup the worksheet and all it contains
    delete m_session;
    m_session=0;
    foreach(WorksheetEntry* entry, m_entries)
        delete entry;
    clear();
    m_entries.clear();

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
        }
        else if (tag == "Text")
        {
            entry = appendTextEntry();
            entry->setContent(expressionChild, file);
        }else if (tag == "Latex")
        {
            entry = appendLatexEntry();
            entry->setContent(expressionChild, file);
        }
	else if (tag == "PageBreak")
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

    int index=m_entries.indexOf(entry);

    m_rootlayout->removeItem(entry);

    delete entry;
    m_entries.removeAll(entry);

    entry = entryAt(index);
    //if (!entry)
    //    entry = entryAt(index + 1);
    if (!entry)
        entry = appendCommandEntry();
    setCurrentEntry(entry);
}


#include "worksheet.moc"
