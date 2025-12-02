/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#include "searchbar.h"

#include "commandentry.h"
#include "worksheet.h"
#include "worksheetentry.h"
#include "worksheettextitem.h"
#include "worksheettexteditoritem.h"
#include "worksheetview.h"

#include <KLocalizedString>
#include <QIcon>
#include <QMenu>

SearchBar::SearchBar(QWidget* parent, Worksheet* worksheet) : QWidget(parent),
    m_stdUi(new Ui::StandardSearchBar()),
    m_worksheet(worksheet),
    m_kateOptions(),
    m_searchFlags{WorksheetEntry::SearchAll}
{
    setupStdUi();
    setStartCursor(worksheet->worksheetCursor());
    setCurrentCursor(m_startCursor);
}

SearchBar::~SearchBar()
{
    if (m_stdUi)
        delete m_stdUi;
    else
        delete m_extUi;
    if (m_currentCursor.isValid()) {
        worksheet()->worksheetView()->setFocus();
        m_currentCursor.entry()->focusEntry();
    } else if (m_startCursor.isValid()) {
        worksheet()->worksheetView()->setFocus();
        m_startCursor.entry()->focusEntry();
    }
}

void SearchBar::showStandard()
{
    if (m_stdUi)
        return;

    delete m_extUi;
    m_extUi = nullptr;
    for (auto* child : children())
        delete child;

    delete layout();
    m_stdUi = new Ui::StandardSearchBar();
    setupStdUi();
}

void SearchBar::showExtended()
{
    if (m_extUi)
        return;

    delete m_stdUi;
    m_stdUi = nullptr;
    for (auto* child : children())
        delete child;

    delete layout();
    m_extUi = new Ui::ExtendedSearchBar();
    setupExtUi();

}

void SearchBar::next()
{
    bool canProceed = m_currentCursor.isValid() || m_currentLegacyCursor.isValid();
    if (!canProceed && !m_atEnd)
        return;
    searchForward(true);
}

void SearchBar::prev()
{
    bool canProceed = m_currentCursor.isValid() || m_currentLegacyCursor.isValid();
    if (!canProceed && !m_atBeginning)
        return;
    searchBackward(true);
}

void SearchBar::setCurrentCursor(KWorksheetCursor cursor)
{
    if (m_currentCursor.entry())
        disconnect(m_currentCursor.entry(), SIGNAL(aboutToBeDeleted()), this, SLOT(invalidateCurrentCursor()));
    if (cursor.entry())
        connect(cursor.entry(), SIGNAL(aboutToBeDeleted()), this, SLOT(invalidateCurrentCursor()), Qt::DirectConnection);

    m_currentCursor = cursor;

    if (cursor.isValid())
        m_currentLegacyCursor = WorksheetCursor();
}

void SearchBar::setCurrentLegacyCursor(WorksheetCursor cursor)
{
    m_currentLegacyCursor = cursor;

    if (cursor.isValid())
        m_currentCursor = KWorksheetCursor();
}

void SearchBar::clearAllCursors()
{
    m_currentCursor = KWorksheetCursor();
    m_currentLegacyCursor = WorksheetCursor();
}

void SearchBar::searchForward(bool skipFirstChar)
{
    worksheet()->clearAllSelections();

    WorksheetEntry* startEntry = nullptr;
    KWorksheetCursor kPos;
    WorksheetCursor legacyPos;

    if (m_currentCursor.isValid())
    {
        startEntry = m_currentCursor.entry();
        kPos = m_currentCursor;
        if (skipFirstChar)
        {
            KTextEditor::Cursor c = kPos.foundRange().end();
            kPos = KWorksheetCursor(startEntry, kPos.textItem(), c);
        }
    }
    else if (m_currentLegacyCursor.isValid())
    {
        startEntry = m_currentLegacyCursor.entry();
        legacyPos = m_currentLegacyCursor;
        if (skipFirstChar)
        {
            QTextCursor c = legacyPos.textCursor();
            c.setPosition(c.selectionEnd());
            legacyPos = WorksheetCursor(startEntry, legacyPos.textItem(), c);
        }
    }
    else
        startEntry = worksheet()->firstEntry();

    if (!startEntry) { setStatus(i18n("Not found")); return; }

    WorksheetEntry* entry = startEntry;
    while (entry)
    {
        bool searchInCommandArea = true;

        if (dynamic_cast<CommandEntry*>(entry) && legacyPos.isValid() && legacyPos.entry() == entry)
            searchInCommandArea = false;

        QGraphicsObject* item = entry->mainTextItem();
        if (item && dynamic_cast<WorksheetTextEditorItem*>(item) && searchInCommandArea)
        {
            KWorksheetCursor result = entry->search(m_pattern, m_searchFlags, m_kateOptions, kPos);
            if (result.isValid())
            {
                m_atEnd = false;
                setCurrentCursor(result);
                worksheet()->setWorksheetCursor(result);
                worksheet()->makeVisible(result);
                clearStatus();
                return;
            }
        }
        else if (item && dynamic_cast<WorksheetTextItem*>(item))
        {
            QTextDocument::FindFlags qtFlags;
            if (!(m_kateOptions & KTextEditor::CaseInsensitive)) { qtFlags |= QTextDocument::FindCaseSensitively; }
            WorksheetCursor result = entry->search(m_pattern, m_searchFlags, qtFlags, legacyPos);
            if (result.isValid())
            {
                m_atEnd = false;
                setCurrentLegacyCursor(result);
                worksheet()->setWorksheetCursor(result);
                worksheet()->makeVisible(result);
                clearStatus();
                return;
            }
        }

        if (auto* commandEntry = dynamic_cast<CommandEntry*>(entry))
        {
            WorksheetCursor startPosForResults = (kPos.isValid() && kPos.entry() == entry) ? WorksheetCursor() : legacyPos;

            QTextDocument::FindFlags qtFlags;
            if (!(m_kateOptions & KTextEditor::CaseInsensitive)) { qtFlags |= QTextDocument::FindCaseSensitively; }

            WorksheetCursor resultInResults = commandEntry->search(m_pattern, m_searchFlags, qtFlags, startPosForResults);
            if (resultInResults.isValid())
            {
                m_atEnd = false;
                setCurrentLegacyCursor(resultInResults);
                worksheet()->setWorksheetCursor(resultInResults);
                worksheet()->makeVisible(resultInResults);
                clearStatus();
                return;
            }
        }

        entry = entry->next();
        kPos = KWorksheetCursor();
        legacyPos = WorksheetCursor();
    }

    if (m_atEnd)
    {
        m_notFound = true;
        setStatus(i18n("Not found"));
        worksheet()->setWorksheetCursor(m_startCursor);
    }
    else
    {
        m_atEnd = true;
        setStatus(i18n("Reached end of document, continuing from top..."));
        clearAllCursors();
        searchForward(false);
    }
}

void SearchBar::searchBackward(bool skipFirstChar)
{
    worksheet()->clearAllSelections();

    WorksheetEntry* startEntry = nullptr;
    KWorksheetCursor kPos;
    WorksheetCursor legacyPos;

    if (m_currentCursor.isValid())
    {
        startEntry = m_currentCursor.entry();
        kPos = m_currentCursor;
        if (skipFirstChar)
        {
            KTextEditor::Cursor c = kPos.foundRange().start();
            kPos = KWorksheetCursor(startEntry, kPos.textItem(), c);
        }
    }
    else if (m_currentLegacyCursor.isValid())
    {
        startEntry = m_currentLegacyCursor.entry();
        legacyPos = m_currentLegacyCursor;
        if (skipFirstChar)
        {
            QTextCursor c = legacyPos.textCursor();
            c.setPosition(c.selectionStart());
            legacyPos = WorksheetCursor(startEntry, legacyPos.textItem(), c);
        }
    }
    else
        startEntry = worksheet()->lastEntry();

    if (!startEntry) { setStatus(i18n("Not found")); return; }

    WorksheetEntry* entry = startEntry;
    while (entry)
    {
        bool searchInResultArea = true;

        if (dynamic_cast<CommandEntry*>(entry) && kPos.isValid() && kPos.entry() == entry)
            searchInResultArea = false;

        if (auto* commandEntry = dynamic_cast<CommandEntry*>(entry))
        {
            if (searchInResultArea)
            {
                WorksheetCursor startPosForResults = (kPos.isValid() && kPos.entry() == entry) ? WorksheetCursor() : legacyPos;

                QTextDocument::FindFlags qtFlagsBackwards = QTextDocument::FindBackward;
                if (!(m_kateOptions & KTextEditor::CaseInsensitive)) { qtFlagsBackwards |= QTextDocument::FindCaseSensitively; }

                WorksheetCursor resultInResults = commandEntry->search(m_pattern, m_searchFlags, qtFlagsBackwards, startPosForResults);
                if (resultInResults.isValid())
                {
                    m_atBeginning = false;
                    setCurrentLegacyCursor(resultInResults);
                    worksheet()->setWorksheetCursor(resultInResults);
                    worksheet()->makeVisible(resultInResults);
                    clearStatus();
                    return;
                }
            }
        }

        QGraphicsObject* item = entry->mainTextItem();
        if (item && dynamic_cast<WorksheetTextEditorItem*>(item))
        {
            KTextEditor::SearchOptions kOptionsBackwards = m_kateOptions | KTextEditor::Backwards;
            KWorksheetCursor result = entry->search(m_pattern, m_searchFlags, kOptionsBackwards, kPos);
            if (result.isValid())
            {
                m_atBeginning = false;
                setCurrentCursor(result);
                worksheet()->setWorksheetCursor(result);
                worksheet()->makeVisible(result);
                clearStatus();
                return;
            }
        }
        else if (item && dynamic_cast<WorksheetTextItem*>(item))
        {
            QTextDocument::FindFlags qtFlagsBackwards = QTextDocument::FindBackward;
            if (!(m_kateOptions & KTextEditor::CaseInsensitive)) { qtFlagsBackwards |= QTextDocument::FindCaseSensitively; }
            WorksheetCursor result = entry->search(m_pattern, m_searchFlags, qtFlagsBackwards, legacyPos);
            if (result.isValid())
            {
                m_atBeginning = false;
                setCurrentLegacyCursor(result);
                worksheet()->setWorksheetCursor(result);
                worksheet()->makeVisible(result);
                clearStatus();
                return;
            }
        }

        entry = entry->previous();
        kPos = KWorksheetCursor();
        legacyPos = WorksheetCursor();
    }

    if (m_atBeginning)
    {
        m_notFound = true;
        setStatus(i18n("Not found"));
        worksheet()->setWorksheetCursor(m_startCursor);
    }
    else
    {
        m_atBeginning = true;
        setStatus(i18n("Reached beginning of document, continuing from bottom..."));
        clearAllCursors();
        searchBackward(false);
    }
}

void SearchBar::handleCloseClicked()
{
    deleteLater();
}

void SearchBar::handleOpenExtendedClicked()
{
    showExtended();
}

void SearchBar::handleOpenStandardClicked()
{
    showStandard();
}

void SearchBar::handleNextClicked()
{
    next();
}

void SearchBar::handlePreviousClicked()
{
    prev();
}

void SearchBar::handleReplaceClicked()
{
    WorksheetEntry* entry = nullptr;
    bool success = false;

    if (m_currentCursor.isValid()) 
        entry = m_currentCursor.entry();
    else if (m_currentLegacyCursor.isValid()) 
        entry = m_currentLegacyCursor.entry();

    if (entry) 
        success = entry->replace(m_replacement);

    if (success) 
        next();
}

void SearchBar::handleReplaceAllClicked()
{
    if (m_pattern.isEmpty())
        return;

    int count = 0;
    for (WorksheetEntry* entry = worksheet()->firstEntry(); entry; entry = entry->next())
    {
        QGraphicsObject* item = entry->mainTextItem();
        if (!item) continue;

        if (auto* editorItem = dynamic_cast<WorksheetTextEditorItem*>(item))
        {
            if (editorItem->isEditable())
            {
                KTextEditor::Cursor searchStartCursor(0, 0);
                while (true)
                {
                    KTextEditor::Range foundRange = editorItem->search(m_pattern, m_kateOptions, searchStartCursor);
                    if (!foundRange.isValid()) break;
                    editorItem->document()->replaceText(foundRange, m_replacement);
                    count++;
                    searchStartCursor.setPosition(foundRange.start().line(), foundRange.start().column() + m_replacement.length());
                }
            }
        }
        else if (auto* textItem = dynamic_cast<WorksheetTextItem*>(item))
        {
            if (textItem->isEditable())
            {
                QTextDocument* doc = textItem->document();
                QTextDocument::FindFlags qtFlags;
                if (!(m_kateOptions & KTextEditor::CaseInsensitive))
                    qtFlags |= QTextDocument::FindCaseSensitively;

                QTextCursor cursor(doc);
                while (true)
                {
                    cursor = doc->find(m_pattern, cursor, qtFlags);
                    if (cursor.isNull()) break;
                    cursor.insertText(m_replacement);
                    count++;
                }
            }
        }
    }
    setStatus(i18np("Replaced %1 instance", "Replaced %1 instances", count));
}


void SearchBar::handlePatternTextChanged(const QString& p)
{
    worksheet()->setWorksheetCursor(KWorksheetCursor());
    m_atBeginning = m_atEnd = m_notFound = false;
    if (!p.startsWith(m_pattern))
        setCurrentCursor(m_startCursor);
    m_pattern = p;
    if (!m_pattern.isEmpty()) {
        searchForward();
        nextButton()->setEnabled(true);
        previousButton()->setEnabled(true);
        if (m_extUi) {
            m_extUi->replace->setEnabled(true);
            m_extUi->replaceAll->setEnabled(true);
        }
    } else {
        worksheet()->setWorksheetCursor(m_startCursor);
        nextButton()->setEnabled(false);
        previousButton()->setEnabled(false);
        if (m_extUi) {
            m_extUi->replace->setEnabled(false);
            m_extUi->replaceAll->setEnabled(false);
        }
    }
}

void SearchBar::handleReplacementTextChanged(const QString& r)
{
    m_replacement = r;
}

void SearchBar::handleRemoveFlagClicked()
{
    QMenu* menu = new QMenu(this);
    fillLocationsMenu(menu, m_searchFlags);
    connect(menu, SIGNAL("aboutToHide()"), menu, SLOT("deleteLater()"));
    menu->exec(mapToGlobal(m_extUi->removeFlag->geometry().topLeft()));
}

void SearchBar::handleAddFlagClicked()
{
    QMenu* menu = new QMenu(this);
    fillLocationsMenu(menu, WorksheetEntry::SearchAll ^ m_searchFlags);
    connect(menu, SIGNAL("aboutToHide()"), menu, SLOT("deleteLater()"));
    menu->exec(mapToGlobal(m_extUi->addFlag->geometry().topLeft()));
}

void SearchBar::invalidateStartCursor()
{
    if (!m_startCursor.isValid())
        return;

    WorksheetEntry* entry = m_startCursor.entry()->next();
    if (!entry && worksheet()->firstEntry() != m_startCursor.entry())
        entry = worksheet()->firstEntry();

    setStartCursor(KWorksheetCursor(entry, nullptr, KTextEditor::Cursor()));
}

void SearchBar::invalidateCurrentCursor()
{
    if (!m_currentCursor.isValid())
        return;

    WorksheetEntry* entry = m_currentCursor.entry()->next();
    if (!entry)
        entry = worksheet()->firstEntry();

    setCurrentCursor(KWorksheetCursor(entry, nullptr, KTextEditor::Cursor()));
}

void SearchBar::toggleFlag()
{
    if (!sender())
        return;
    int flag = sender()->property("searchFlag").toInt();
    m_searchFlags ^= flag;
    updateSearchLocations();
}

void SearchBar::handleMatchCaseToggled(bool b)
{
    if(b)
        m_kateOptions &= ~KTextEditor::SearchOption::CaseInsensitive;
    else
        m_kateOptions |= KTextEditor::SearchOption::CaseInsensitive;
    searchForward();
}

void SearchBar::updateSearchLocations()
{
    static QList<QString> names;
    if (names.empty())
        names << i18n("Commands") << i18n("Results") << i18n("Errors")
              << i18n("Text") << i18n("LaTeX Code");

    QString text = QLatin1String("");
    int flag = 1;
    for (int i = 0; flag < WorksheetEntry::SearchAll; flag = (1<<(++i))) {
        if (m_searchFlags & flag) {
            if (!text.isEmpty())
                text += QLatin1String(", ");
            text += names.at(i);
        }
    }
    m_extUi->searchFlagsList->setText(text);
    if (m_searchFlags == 0) {
        m_extUi->removeFlag->setEnabled(false);
        m_extUi->addFlag->setEnabled(true);
    } else if (m_searchFlags == WorksheetEntry::SearchAll) {
        m_extUi->removeFlag->setEnabled(true);
        m_extUi->addFlag->setEnabled(false);
    } else {
        m_extUi->addFlag->setEnabled(true);
        m_extUi->removeFlag->setEnabled(true);
    }
}

void SearchBar::fillLocationsMenu(QMenu* menu, int flags)
{
    static QList<QString> names;
    if (names.empty())
        names << i18n("Commands") << i18n("Results") << i18n("Errors")
              << i18n("Text") << i18n("LaTeX Code");
    int flag = 1;
    for (int i = 0; flag < WorksheetEntry::SearchAll; flag = (1<<(++i))) {
        if (flags & flag) {
            QAction* a = menu->addAction(names.at(i), this, SLOT(toggleFlag()));
            a->setProperty("searchFlag", flag);
        }
    }
}

void SearchBar::setStartCursor(KWorksheetCursor cursor)
{
    if (m_startCursor.entry())
        disconnect(m_startCursor.entry(), SIGNAL(aboutToBeDeleted()), this,
                   SLOT(invalidateStartCursor()));
    if (cursor.entry())
        connect(cursor.entry(), SIGNAL(aboutToBeDeleted()), this,
                SLOT(invalidateStartCursor()), Qt::DirectConnection);
    m_startCursor = cursor;
}

void SearchBar::setStatus(QString message)
{
    KSqueezedTextLabel* status;
    if (m_stdUi)
        status = m_stdUi->status;
    else
        status = m_extUi->status;

    status->setText(message);
}

void SearchBar::clearStatus()
{
    setStatus(QLatin1String(""));
}

void SearchBar::setupStdUi()
{
    if (!m_stdUi)
        return;

    m_stdUi->setupUi(this);
    connect(m_stdUi->close, &QToolButton::clicked, this, &SearchBar::handleCloseClicked);
    connect(m_stdUi->openExtended, &QToolButton::clicked, this, &SearchBar::handleOpenExtendedClicked);
    connect(m_stdUi->next, &QPushButton::clicked, this, &SearchBar::handleNextClicked);
    connect(m_stdUi->previous, &QPushButton::clicked, this, &SearchBar::handlePreviousClicked);
    connect(m_stdUi->pattern, &KLineEdit::textChanged, this, &SearchBar::handlePatternTextChanged);
    connect(m_stdUi->matchCase, &QToolButton::toggled, this, &SearchBar::handleMatchCaseToggled);
    if (m_pattern.isEmpty()) {
        m_stdUi->next->setEnabled(false);
        m_stdUi->previous->setEnabled(false);
    }

    m_stdUi->close->setShortcut(Qt::Key_Escape);
    setFocusProxy(m_stdUi->pattern);
}

void SearchBar::setupExtUi()
{
    if (!m_extUi)
        return;

    m_extUi->setupUi(this);
    connect(m_extUi->close, &QToolButton::clicked, this, &SearchBar::handleCloseClicked);
    connect(m_extUi->openStandard, &QToolButton::clicked, this, &SearchBar::handleOpenStandardClicked);
    connect(m_extUi->next, &QPushButton::clicked, this, &SearchBar::handleNextClicked);
    connect(m_extUi->previous, &QPushButton::clicked, this, &SearchBar::handlePreviousClicked);
    connect(m_extUi->replace, &QPushButton::clicked, this, &SearchBar::handleReplaceClicked);
    connect(m_extUi->replaceAll, &QPushButton::clicked, this, &SearchBar::handleReplaceAllClicked);
    connect(m_extUi->pattern, &KLineEdit::textChanged, this, &SearchBar::handlePatternTextChanged);
    connect(m_extUi->replacement, &KLineEdit::textChanged, this, &SearchBar::handleReplacementTextChanged);
    connect(m_extUi->addFlag, &QToolButton::clicked, this, &SearchBar::handleAddFlagClicked);
    connect(m_extUi->removeFlag, &QToolButton::clicked, this, &SearchBar::handleRemoveFlagClicked);
    connect(m_extUi->matchCase, &QCheckBox::toggled, this, &SearchBar::handleMatchCaseToggled);
    if (m_pattern.isEmpty()) {
        m_extUi->next->setEnabled(false);
        m_extUi->previous->setEnabled(false);
        m_extUi->replace->setEnabled(false);
        m_extUi->replaceAll->setEnabled(false);
    }

    m_extUi->addFlag->setIcon(QIcon::fromTheme(QLatin1String("list-add")));
    m_extUi->removeFlag->setIcon(QIcon::fromTheme(QLatin1String("list-remove")));

    m_extUi->close->setShortcut(Qt::Key_Escape);
    setFocusProxy(m_extUi->pattern);
    updateSearchLocations();
}

QPushButton* SearchBar::previousButton()
{
    if (m_stdUi)
        return m_stdUi->previous;
    else
        return m_extUi->previous;
}

QPushButton* SearchBar::nextButton()
{
    if (m_stdUi)
        return m_stdUi->next;
    else
        return m_extUi->next;
}

Worksheet* SearchBar::worksheet()
{
    return m_worksheet;
}
