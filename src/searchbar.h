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

#ifndef SEARCHBAR_H
#define SEARCHBAR_H

#include <QWidget>
#include <QList>
#include <QTextDocument>

#include "ui_standardsearchbar.h"
#include "ui_extendedsearchbar.h"

#include "worksheetcursor.h"

class Worksheet;
class WorksheetEntry;
class WorksheetTextItem;

class SearchBar : public QWidget
{
  Q_OBJECT;
  public:
    SearchBar(QWidget* parent, Worksheet* worksheet);
    ~SearchBar();

    void showStandard();
    void showExtended();

    void next();
    void prev();

    void searchForward(bool skipFirstChar = false);
    void searchBackward(bool skipFirstChar = false);

  public slots:
    void on_close_clicked();
    void on_openExtended_clicked();
    void on_openStandard_clicked();
    void on_next_clicked();
    void on_previous_clicked();
    void on_replace_clicked();
    void on_replaceAll_clicked();
    void on_pattern_textChanged(const QString& p);
    void on_replacement_textChanged(const QString& r);
    void on_matchCase_toggled(bool b);

  private:

    void setupStdUi();
    void setupExtUi();

    void setStatus(QString);
    void clearStatus();

    Worksheet* worksheet();

    Ui::StandardSearchBar* m_stdUi;
    Ui::ExtendedSearchBar* m_extUi;

    WorksheetCursor m_startCursor;
    WorksheetCursor m_currentCursor;

    Worksheet* m_worksheet;
    QString m_pattern;
    QString m_replacement;
    QTextDocument::FindFlags m_qtFlags;
    unsigned int m_searchFlags;

    bool m_atBeginning;
    bool m_atEnd;
    bool m_notFound;
};

#endif // SEARCHBAR_H

