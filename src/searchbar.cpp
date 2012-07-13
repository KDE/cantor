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
    Copyright (C) 2012 Martin Kuettler <martin.kuettler@gmail.com>
 */

#include "searchbar.h"

#include "worksheet.h"
#include "worksheetentry.h"
#include "worksheettextitem.h"

#include <KIcon>
#include <KDebug>

SearchBar::SearchBar(QWidget* parent, Worksheet* worksheet) : QWidget(parent)
{
    m_worksheet = worksheet;
    m_stdUi = new Ui::StandardSearchBar();
    m_extUi = 0;
    setupStdUi();
    m_qtFlags = 0;
    m_startCursor = worksheet->worksheetCursor();
    m_currentCursor = m_startCursor;
    m_atBeginning = false;
    m_atEnd = false;
    m_notFound = false;
    m_searchFlags = WorksheetEntry::SearchCommand |
	WorksheetEntry::SearchError | WorksheetEntry::SearchResult |
	WorksheetEntry::SearchText | WorksheetEntry::SearchLaTeX;
}

SearchBar::~SearchBar()
{
    if (m_stdUi)
	delete m_stdUi;
    else
	delete m_extUi;
}

void SearchBar::showStandard()
{
    if (m_stdUi)
	return;

    delete m_extUi;
    m_extUi = 0;
    foreach(QObject* child, children()) {
	child->deleteLater();
    }
    delete layout();
    m_stdUi = new Ui::StandardSearchBar();
    setupStdUi();
}

void SearchBar::showExtended()
{
    if (m_extUi)
	return;

    delete m_stdUi;
    m_stdUi = 0;
    foreach(QObject* child, children()) {
	delete child;
    }
    delete layout();
    m_extUi = new Ui::ExtendedSearchBar();
    setupExtUi();

}

void SearchBar::next()
{
    if (!m_currentCursor.isValid() && !m_atEnd)
	return;
    searchForward(true);
}

void SearchBar::prev()
{
    if (!m_currentCursor.isValid() && !m_atBeginning)
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
	    m_currentCursor = WorksheetCursor(m_currentCursor.entry(),
					      m_currentCursor.textItem(), c);
	}
	if (!atBeginningOfEntry)
	    result = m_currentCursor.entry()->search(m_pattern, m_searchFlags,
						 f, m_currentCursor);
	entry = m_currentCursor.entry()->previous();
    } else {
	entry = worksheet()->lastEntry();
    }
    m_currentCursor = WorksheetCursor();

    while (!result.isValid() && entry) {
	result = entry->search(m_pattern, m_searchFlags, f);
	entry = entry->previous();
    }
    if (result.isValid()) {
	m_atBeginning = false;
	QTextCursor c = result.textCursor();
	if (result.textCursor().hasSelection())
	    c.setPosition(result.textCursor().selectionStart());
	m_currentCursor = WorksheetCursor(result.entry(), result.textItem(), c);
	clearStatus();
	worksheet()->setWorksheetCursor(result);
    } else {
	if (m_atBeginning) {
	    m_notFound = true;
	    setStatus(i18n("Not found"));
	} else {
	    m_atBeginning = true;
	    setStatus(i18n("Reached beginning of worksheet"));
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
	    kDebug() << c.position();
	    m_currentCursor = WorksheetCursor(m_currentCursor.entry(),
					      m_currentCursor.textItem(), c);
	}
	result = m_currentCursor.entry()->search(m_pattern, m_searchFlags,
						 m_qtFlags, m_currentCursor);
	entry = m_currentCursor.entry()->next();
    } else {
	entry = worksheet()->firstEntry();
    }
    m_currentCursor = WorksheetCursor();

    while (!result.isValid() && entry) {
	result = entry->search(m_pattern, m_searchFlags, m_qtFlags);
	entry = entry->next();
    }

    if (result.isValid()) {
	m_atEnd = false;
	QTextCursor c = result.textCursor();
	if (result.textCursor().hasSelection())
	    c.setPosition(result.textCursor().selectionStart());
	m_currentCursor = WorksheetCursor(result.entry(), result.textItem(), c);
	clearStatus();
	worksheet()->setWorksheetCursor(result);
    } else {
	if (m_atEnd) {
	    m_notFound = true;
	    setStatus(i18n("Not found"));
	} else {
	    m_atEnd = true;
	    setStatus(i18n("Reached end of worksheet"));
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
    setStatus(i18n("Replaced %1 instances").arg(count));
}

void SearchBar::on_pattern_textChanged(const QString& p)
{
    worksheet()->setWorksheetCursor(WorksheetCursor());
    m_atBeginning = m_atEnd = m_notFound = false;
    if (!p.startsWith(m_pattern))
	m_currentCursor = m_startCursor;
    m_pattern = p;
    if (!m_pattern.isEmpty())
	searchForward();
    else
	worksheet()->setWorksheetCursor(m_startCursor);
}

void SearchBar::on_replacement_textChanged(const QString& r)
{
    m_replacement = r;
}

void SearchBar::on_matchCase_toggled(bool b)
{
    m_qtFlags &= ~QTextDocument::FindCaseSensitively;
    if (b)
	m_qtFlags |= QTextDocument::FindCaseSensitively;
    searchForward();
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
    setStatus("");
}

void SearchBar::setupStdUi()
{
    if (!m_stdUi)
	return;

    m_stdUi->setupUi(this);
    m_stdUi->close->setIcon(KIcon("dialog-close"));
    m_stdUi->openExtended->setIcon(KIcon("arrow-up-double"));
    m_stdUi->pattern->setText(m_pattern);
    m_stdUi->matchCase->setChecked(m_qtFlags & QTextDocument::FindCaseSensitively);
    m_stdUi->next->setIcon(KIcon("go-down-search"));
    m_stdUi->previous->setIcon(KIcon("go-up-search"));

    setFocusProxy(m_stdUi->pattern);
}

void SearchBar::setupExtUi()
{
    if (!m_extUi)
	return;

    m_extUi->setupUi(this);
    m_extUi->close->setIcon(KIcon("dialog-close"));
    m_extUi->openStandard->setIcon(KIcon("arrow-down-double"));
    m_extUi->pattern->setText(m_pattern);
    m_extUi->replacement->setText(m_replacement);
    m_extUi->matchCase->setChecked(m_qtFlags & QTextDocument::FindCaseSensitively);
    m_extUi->next->setIcon(KIcon("go-down-search"));
    m_extUi->previous->setIcon(KIcon("go-up-search"));
    // ...

    setFocusProxy(m_extUi->pattern);
}

Worksheet* SearchBar::worksheet()
{
    return m_worksheet;
}

#include "searchbar.moc"
