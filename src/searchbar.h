/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#ifndef SEARCHBAR_H
#define SEARCHBAR_H

#include <KTextEditor/Document>

#include <QWidget>
#include <QTextDocument>

#include "ui_standardsearchbar.h"
#include "ui_extendedsearchbar.h"

#include "worksheetcursor.h"

class Worksheet;
class WorksheetEntry;
class WorksheetTextItem;
class WorksheetTextEditorItem;

class QMenu;

class SearchBar : public QWidget
{
  Q_OBJECT
  public:
    SearchBar(QWidget*, Worksheet*);
    ~SearchBar() override;

    void showStandard();
    void showExtended();

    void next();
    void prev();

    void searchForward(bool skipFirstChar = false);
    void searchBackward(bool skipFirstChar = false);

  public Q_SLOTS:
    void handleCloseClicked();
    void handleOpenExtendedClicked();
    void handleOpenStandardClicked();
    void handleNextClicked();
    void handlePreviousClicked();
    void handleReplaceClicked();
    void handleReplaceAllClicked();
    void handlePatternTextChanged(const QString&);
    void handleReplacementTextChanged(const QString&);
    void handleAddFlagClicked();
    void handleRemoveFlagClicked();
    void handleMatchCaseToggled(bool b);

    void invalidateStartCursor();
    void invalidateCurrentCursor();

  protected Q_SLOTS:
    void toggleFlag();

  private:

    void updateSearchLocations();
    void fillLocationsMenu(QMenu*, int flags);

    void setupStdUi();
    void setupExtUi();

    void setStatus(QString);
    void clearStatus();

    void setStartCursor(KWorksheetCursor);
    void setCurrentCursor(KWorksheetCursor);
    void setCurrentLegacyCursor(WorksheetCursor);
    void clearAllCursors();

    Worksheet* worksheet();

    QPushButton* nextButton();
    QPushButton* previousButton();

  private:
    Ui::StandardSearchBar* m_stdUi{nullptr};
    Ui::ExtendedSearchBar* m_extUi{nullptr};

    KWorksheetCursor m_startCursor;
    KWorksheetCursor m_currentCursor;
    WorksheetCursor m_currentLegacyCursor;

    Worksheet* m_worksheet;
    QString m_pattern;
    QString m_replacement;
    QTextDocument::FindFlags m_qtFlags;
    KTextEditor::SearchOptions m_kateOptions;
    unsigned int m_searchFlags;

    bool m_atBeginning{false};
    bool m_atEnd{false};
    bool m_notFound{false};
};

#endif // SEARCHBAR_H

