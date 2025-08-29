/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
    SPDX-FileCopyrightText: 2018-2021 Alexander Semke <alexander.semke@web.de>
*/

#ifndef COMMANDENTRY_H
#define COMMANDENTRY_H

#include <QPointer>
#include <QTimer>

#include "worksheetentry.h"
#include "lib/expression.h"
#include "dynamichighlighter.h"

class Worksheet;
class ResultItem;
class QTimer;
class QJsonObject;

namespace Cantor{
    class Result;
}

class CommandEntry : public WorksheetEntry
{
  Q_OBJECT
  public:
    static const QString Prompt;
    static const QString MidPrompt;
    static const QString HidePrompt;

    explicit CommandEntry(Worksheet*);
    ~CommandEntry() override;

    enum {Type = UserType + 2};
    int type() const override;

    QString command();
    void setExpression(Cantor::Expression*);
    Cantor::Expression* expression();

    QString currentLine();

    bool isEmpty() override;
    bool isExcludedFromExecution();
    bool isResultCollapsed();

    void setContent(const QString&) override;
    void setContent(const QDomElement&, const KZip&) override;
    void setContentFromJupyter(const QJsonObject&) override;

    QDomElement toXml(QDomDocument&, KZip*) override;
    QJsonValue toJupyterJson() override;
    QString toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq) override;


    bool acceptRichText() override;
    void setVariableHighlightingEnabled(bool enabled);

    void interruptEvaluation() override;


    bool focusEntry(int pos = WorksheetTextEditorItem::TopLeft, qreal xCoord = 0) override;

    void layOutForWidth(qreal entry_zone_x, qreal w, bool force = false) override;
    qreal promptItemWidth();

    WorksheetTextEditorItem* highlightItem() override;

    QGraphicsObject* mainTextItem() const override;

    WorksheetCursor search(const QString& pattern, unsigned flags,
                           QTextDocument::FindFlags qt_flags,
                           const WorksheetCursor& pos = WorksheetCursor()) override;
    KWorksheetCursor search(const QString& pattern, unsigned flags,
                            KTextEditor::SearchOptions options,
                            const KWorksheetCursor& pos = KWorksheetCursor()) override;
    bool replace(const QString& replacement) override;
    void replaceAll(const QString& pattern, const QString& replacement, unsigned flags, KTextEditor::SearchOptions options);

  public Q_SLOTS:
    bool evaluateCurrentItem() override;
    bool evaluate(WorksheetEntry::EvaluationOption evalOp = FocusNext) override;
    void addInformation();
    void removeResults();
    void removeResult(Cantor::Result* result);
    void collapseResults();
    void expandResults();
    void excludeFromExecution();
    void addToExecution();


    void updateEntry() override;
    void updatePrompt(const QString& postfix = CommandEntry::Prompt);
    void expressionChangedStatus(Cantor::Expression::Status);
    void showAdditionalInformationPrompt(const QString&);

    void startRemoving(bool warn = true) override;

    void moveToNextItem(int pos, qreal x);
    void moveToPreviousItem(int pos, qreal x);

    void populateMenu(QMenu*, QPointF) override;
    void updateAfterSettingsChanges() override;
  protected:
    bool wantToEvaluate() override;

  private:
    WorksheetTextEditorItem* currentInformationItem();
    bool informationItemHasFocus();
    bool focusWithinThisItem();

    QPoint toGlobalPosition(QPointF);
    void initMenus();


    enum CompletionMode {PreliminaryCompletion, FinalCompletion};
    static const double VerticalSpacing;

    WorksheetTextEditorItem* m_promptItem;
    WorksheetTextEditorItem* m_commandItem;
    QVector<ResultItem*> m_resultItems;
    bool m_resultsCollapsed;
    QList<WorksheetTextEditorItem*> m_informationItems;
    Cantor::Expression* m_expression;

    DynamicHighlighter* m_dynamicHighlighter;
    bool m_variableHighlightingEnabled = true;

    EvaluationOption m_evaluationOption;
    QPropertyAnimation* m_promptItemAnimation;
    bool m_menusInitialized;
    bool m_textColorCustom;
    bool m_backgroundColorCustom;

    //formatting
    QActionGroup* m_backgroundColorActionGroup;
    QMenu* m_backgroundColorMenu;
    QActionGroup* m_textColorActionGroup;
    QActionGroup* m_themeActionGroup;
    QColor m_defaultDefaultTextColor;
    QMenu* m_textColorMenu;
    QMenu* m_themeMenu;
    QMenu* m_fontMenu;

    bool m_isExecutionEnabled;
    QColor m_activeExecutionTextColor;
    QColor m_activeExecutionBackgroundColor;

  private Q_SLOTS:
    void invalidate();
    void resultDeleted();
    void clearResultItems();
    void removeResultItem(int index);
    void replaceResultItem(int index);

    void changeResultCollapsingAction();
    void showHelp();
    void toggleEnabled();

    void backgroundColorChanged(QAction*);
    void textColorChanged(QAction*);
    void themeChanged(QAction*);
    void fontBoldTriggered();
    void fontItalicTriggered();
    void fontIncreaseTriggered();
    void fontDecreaseTriggered();
    void fontSelectTriggered();
    void resetFontTriggered();

    void animatePromptItem();
    void setMidPrompt();
    void setHidePrompt();
};

#endif // COMMANDENTRY_H
