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
    m_startCursor = worksheet->worksheetCursor();
    m_searchFlags = WorksheetEntry::SearchText;
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

}

void SearchBar::prev()
{
}

void SearchBar::search()
{
    WorksheetCursor result;
    WorksheetEntry* entry;
    if (m_startCursor.isValid()) {
	result = m_startCursor.entry()->search(m_pattern, m_searchFlags,
					       m_startCursor);
	entry = m_startCursor.entry()->next();
    } else {
	entry = worksheet()->firstEntry();
    }

    while (!result.isValid() && entry) {
	result = entry->search(m_pattern, m_searchFlags);
	entry = entry->next();
    }
    
    if (result.isValid()) {
	m_currentCursor = result;
	clearStatus();
	worksheet()->setWorksheetCursor(result);
    } else {
	setStatus(i18n("Not found"));
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

void SearchBar::on_pattern_textChanged(const QString& p)
{
    worksheet()->setWorksheetCursor(WorksheetCursor());
    if (!p.startsWith(m_pattern))
	m_currentCursor = m_startCursor;
    m_pattern = p;
    if (!m_pattern.isEmpty())
	search();
}

void SearchBar::on_replace_textChanged(const QString& r)
{
    m_replace = r;
}

void SearchBar::on_matchCase_toggled(bool b)
{
    m_matchCase = b;
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
    m_stdUi->matchCase->setChecked(m_matchCase);
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
    m_extUi->replace->setText(m_replace);
    m_extUi->matchCase->setChecked(m_matchCase);
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
