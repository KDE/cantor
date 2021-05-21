/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
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

class QMenu;

class SearchBar : public QWidget
{
  Q_OBJECT
  public:
    SearchBar(QWidget* parent, Worksheet* worksheet);
    ~SearchBar() override;

    void showStandard();
    void showExtended();

    void next();
    void prev();

    void searchForward(bool skipFirstChar = false);
    void searchBackward(bool skipFirstChar = false);

  public Q_SLOTS:
    void on_close_clicked();
    void on_openExtended_clicked();
    void on_openStandard_clicked();
    void on_next_clicked();
    void on_previous_clicked();
    void on_replace_clicked();
    void on_replaceAll_clicked();
    void on_pattern_textChanged(const QString& p);
    void on_replacement_textChanged(const QString& r);
    void on_addFlag_clicked();
    void on_removeFlag_clicked();
    void on_matchCase_toggled(bool b);

    void invalidateStartCursor();
    void invalidateCurrentCursor();

  protected Q_SLOTS:
    void toggleFlag();

  private:

    void updateSearchLocations();
    void fillLocationsMenu(QMenu* menu, int flags);

    void setupStdUi();
    void setupExtUi();

    void setStatus(QString);
    void clearStatus();

    void setStartCursor(WorksheetCursor cursor);
    void setCurrentCursor(WorksheetCursor cursor);

    Worksheet* worksheet();

    QPushButton* nextButton();
    QPushButton* previousButton();

  private:
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

