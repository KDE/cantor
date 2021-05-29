/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#include "searchbar.h"

#include "worksheet.h"
#include "worksheetentry.h"
#include "worksheettextitem.h"
#include "worksheetview.h"

#include <KLocalizedString>
#include <QIcon>
#include <QMenu>

SearchBar::SearchBar(QWidget* parent, Worksheet* worksheet) : QWidget(parent),
    m_stdUi(new Ui::StandardSearchBar()),
    m_worksheet(worksheet),
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
    if (!m_currentCursor.isValid() && !m_currentCursor.entry() && !m_atEnd)
        return;
    searchForward(true);
}

void SearchBar::prev()
{
    if (!m_currentCursor.isValid() && !m_currentCursor.entry() &&
        !m_atBeginning)
        return;
    searchBackward(true);
}

void SearchBar::searchBackward(bool skipFirstChar)
{
    WorksheetCursor result;
    WorksheetEntry* entry;
    worksheet()->setWorksheetCursor(WorksheetCursor());
    QTextDocument::FindFlags f = m_qtFlags | QTextDocument::FindBackward;
    if (m_currentCursor.isValid()) {
        bool atBeginningOfEntry = false;
        if (skipFirstChar) {
            QTextCursor c = m_currentCursor.textCursor();
            c.movePosition(QTextCursor::PreviousCharacter);
            atBeginningOfEntry = (c == m_currentCursor.textCursor());
            setCurrentCursor(WorksheetCursor(m_currentCursor.entry(),
                                             m_currentCursor.textItem(), c));
        }
        if (!atBeginningOfEntry)
            result = m_currentCursor.entry()->search(m_pattern, m_searchFlags,
                                                 f, m_currentCursor);
        entry = m_currentCursor.entry()->previous();
    } else if (m_currentCursor.entry() && m_currentCursor.entry()->previous()) {
        entry = m_currentCursor.entry()->previous();
    } else {
        entry = worksheet()->lastEntry();
    }
    setCurrentCursor(WorksheetCursor());

    while (!result.isValid() && entry) {
        result = entry->search(m_pattern, m_searchFlags, f);
        entry = entry->previous();
    }
    if (result.isValid()) {
        m_atBeginning = false;
        QTextCursor c = result.textCursor();
        if (result.textCursor().hasSelection())
            c.setPosition(result.textCursor().selectionStart());
        setCurrentCursor(WorksheetCursor(result.entry(), result.textItem(), c));
        worksheet()->makeVisible(m_currentCursor);
        clearStatus();
        worksheet()->setWorksheetCursor(result);
    } else {
        if (m_atBeginning) {
            m_notFound = true;
            setStatus(i18n("Not found"));
        } else {
            m_atBeginning = true;
            setStatus(i18n("Reached beginning"));
        }
        worksheet()->setWorksheetCursor(m_startCursor);
    }
}

void SearchBar::searchForward(bool skipFirstChar)
{
    WorksheetCursor result;
    WorksheetEntry* entry;
    worksheet()->setWorksheetCursor(WorksheetCursor());
    if (m_currentCursor.isValid()) {
        if (skipFirstChar) {
            QTextCursor c = m_currentCursor.textCursor();
            c.movePosition(QTextCursor::NextCharacter);
            setCurrentCursor(WorksheetCursor(m_currentCursor.entry(),
                                             m_currentCursor.textItem(), c));
        }
        result = m_currentCursor.entry()->search(m_pattern, m_searchFlags,
                                                 m_qtFlags, m_currentCursor);
        entry = m_currentCursor.entry()->next();
    } else if (m_currentCursor.entry()) {
        entry = m_currentCursor.entry();
    } else {
        entry = worksheet()->firstEntry();
    }
    setCurrentCursor(WorksheetCursor());

    while (!result.isValid() && entry) {
        result = entry->search(m_pattern, m_searchFlags, m_qtFlags);
        entry = entry->next();
    }

    if (result.isValid()) {
        m_atEnd = false;
        QTextCursor c = result.textCursor();
        if (result.textCursor().hasSelection())
            c.setPosition(result.textCursor().selectionStart());
        setCurrentCursor(WorksheetCursor(result.entry(), result.textItem(), c));
        worksheet()->makeVisible(m_currentCursor);
        clearStatus();
        worksheet()->setWorksheetCursor(result);
    } else {
        if (m_atEnd) {
            m_notFound = true;
            setStatus(i18n("Not found"));
        } else {
            m_atEnd = true;
            setStatus(i18n("Reached end"));
        }
        worksheet()->setWorksheetCursor(m_startCursor);
    }
}

void SearchBar::on_close_clicked()
{
    deleteLater();
}

void SearchBar::on_openExtended_clicked()
{
    showExtended();
}

void SearchBar::on_openStandard_clicked()
{
    showStandard();
}

void SearchBar::on_next_clicked()
{
    next();
}

void SearchBar::on_previous_clicked()
{
    prev();
}

void SearchBar::on_replace_clicked()
{
    if (!m_currentCursor.isValid())
        return;

    QTextCursor cursor = m_currentCursor.textCursor();
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor,
                        m_pattern.length());
    cursor.insertText(m_replacement);
    next();
}

void SearchBar::on_replaceAll_clicked()
{
    int count = 0;
    WorksheetEntry* entry = worksheet()->firstEntry();
    WorksheetCursor cursor;
    for (; entry; entry = entry->next()) {
        cursor = entry->search(m_pattern, m_searchFlags, m_qtFlags);
        while (cursor.isValid()) {
            cursor.textCursor().insertText(m_replacement);
            cursor = entry->search(m_pattern, m_searchFlags, m_qtFlags,
                                   cursor);
            ++count;
        }
    }
    setStatus(i18np("Replaced %1 instance", "Replaced %1 instances", count));
}

void SearchBar::on_pattern_textChanged(const QString& p)
{
    worksheet()->setWorksheetCursor(WorksheetCursor());
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

void SearchBar::on_replacement_textChanged(const QString& r)
{
    m_replacement = r;
}

void SearchBar::on_removeFlag_clicked()
{
    QMenu* menu = new QMenu(this);
    fillLocationsMenu(menu, m_searchFlags);
    connect(menu, SIGNAL("aboutToHide()"), menu, SLOT("deleteLater()"));
    menu->exec(mapToGlobal(m_extUi->removeFlag->geometry().topLeft()));
}

void SearchBar::on_addFlag_clicked()
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

    setStartCursor(WorksheetCursor(entry, nullptr, QTextCursor()));
}

void SearchBar::invalidateCurrentCursor()
{
    if (!m_currentCursor.isValid())
        return;

    WorksheetEntry* entry = m_currentCursor.entry()->next();
    if (!entry)
        entry = worksheet()->firstEntry();

    setCurrentCursor(WorksheetCursor(entry, nullptr, QTextCursor()));
}

void SearchBar::toggleFlag()
{
    if (!sender())
        return;
    int flag = sender()->property("searchFlag").toInt();
    m_searchFlags ^= flag;
    updateSearchLocations();
}

void SearchBar::on_matchCase_toggled(bool b)
{
    m_qtFlags &= ~QTextDocument::FindCaseSensitively;
    if (b)
        m_qtFlags |= QTextDocument::FindCaseSensitively;
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

void SearchBar::setStartCursor(WorksheetCursor cursor)
{
    if (m_startCursor.entry())
        disconnect(m_startCursor.entry(), SIGNAL(aboutToBeDeleted()), this,
                   SLOT(invalidateStartCursor()));
    if (cursor.entry())
        connect(cursor.entry(), SIGNAL(aboutToBeDeleted()), this,
                SLOT(invalidateStartCursor()), Qt::DirectConnection);
    m_startCursor = cursor;
}

void SearchBar::setCurrentCursor(WorksheetCursor cursor)
{
    if (m_currentCursor.entry())
        disconnect(m_currentCursor.entry(), SIGNAL(aboutToBeDeleted()), this,
                   SLOT(invalidateCurrentCursor()));
    if (cursor.entry())
        connect(cursor.entry(), SIGNAL(aboutToBeDeleted()), this,
                SLOT(invalidateCurrentCursor()), Qt::DirectConnection);
    m_currentCursor = cursor;
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
    m_stdUi->close->setIcon(QIcon::fromTheme(QLatin1String("dialog-close")));
    m_stdUi->openExtended->setIcon(QIcon::fromTheme(QLatin1String("arrow-up-double")));
    m_stdUi->pattern->setText(m_pattern);
    m_stdUi->matchCase->setChecked(m_qtFlags & QTextDocument::FindCaseSensitively);
    m_stdUi->next->setIcon(QIcon::fromTheme(QLatin1String("go-down-search")));
    m_stdUi->previous->setIcon(QIcon::fromTheme(QLatin1String("go-up-search")));
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
    m_extUi->close->setIcon(QIcon::fromTheme(QLatin1String("dialog-close")));
    m_extUi->openStandard->setIcon(QIcon::fromTheme(QLatin1String("arrow-down-double")));
    m_extUi->pattern->setText(m_pattern);
    m_extUi->replacement->setText(m_replacement);
    m_extUi->matchCase->setChecked(m_qtFlags & QTextDocument::FindCaseSensitively);
    m_extUi->next->setIcon(QIcon::fromTheme(QLatin1String("go-down-search")));
    m_extUi->previous->setIcon(QIcon::fromTheme(QLatin1String("go-up-search")));
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
