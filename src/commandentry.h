/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
    SPDX-FileCopyrightText: 2018-2021 Alexander Semke <alexander.semke@web.de>
*/

#ifndef COMMANDENTRY_H
#define COMMANDENTRY_H

#include <QPointer>
#include <KCompletionBox>

#include "worksheetentry.h"
#include "lib/expression.h"

class Worksheet;
class ResultItem;
class QTimer;
class QJsonObject;

namespace Cantor{
    class Result;
    class CompletionObject;
    class SyntaxHelpObject;
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

    void setCompletion(Cantor::CompletionObject* tc);
    void setSyntaxHelp(Cantor::SyntaxHelpObject* sh);

    bool acceptRichText() override;

    void removeContextHelp();

    void interruptEvaluation() override;
    bool isShowingCompletionPopup();

    bool focusEntry(int pos = WorksheetTextItem::TopLeft, qreal xCoord = 0) override;

    void layOutForWidth(qreal entry_zone_x, qreal w, bool force = false) override;
    qreal promptItemWidth();

    WorksheetTextItem* highlightItem() override;

    WorksheetCursor search(const QString& pattern, unsigned flags,
                           QTextDocument::FindFlags qt_flags,
                           const WorksheetCursor& pos = WorksheetCursor()) override;

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

    void showCompletion() override;
    void handleTabPress();
    void handleBacktabPress();
    void updateEntry() override;
    void updatePrompt(const QString& postfix = CommandEntry::Prompt);
    void expressionChangedStatus(Cantor::Expression::Status);
    void showAdditionalInformationPrompt(const QString&);
    void showCompletions();
    void applySelectedCompletion();
    void completedLineChanged();
    void showSyntaxHelp();
    void completeLineTo(const QString& line, int index);

    void startRemoving(bool warn = true) override;

    void moveToNextItem(int pos, qreal x);
    void moveToPreviousItem(int pos, qreal x);

    void populateMenu(QMenu*, QPointF) override;

  protected:
    bool wantToEvaluate() override;

  private:
    WorksheetTextItem* currentInformationItem();
    bool informationItemHasFocus();
    bool focusWithinThisItem();
    QPoint getPopupPosition();
    QPoint toGlobalPosition(QPointF);
    void initMenus();
    void handleExistedCompletionBox();
    void makeCompletion(const QString& line, int position);

    enum CompletionMode {PreliminaryCompletion, FinalCompletion};
    static const double VerticalSpacing;

    WorksheetTextItem* m_promptItem;
    WorksheetTextItem* m_commandItem;
    QVector<ResultItem*> m_resultItems;
    bool m_resultsCollapsed;
    WorksheetTextItem* m_errorItem;
    QList<WorksheetTextItem*> m_informationItems;
    Cantor::Expression* m_expression;

    Cantor::CompletionObject* m_completionObject;
    QPointer<KCompletionBox> m_completionBox;
    Cantor::SyntaxHelpObject* m_syntaxHelpObject;

    EvaluationOption m_evaluationOption;
    QPropertyAnimation* m_promptItemAnimation;
    bool m_menusInitialized;
    bool m_textColorCustom;
    bool m_backgroundColorCustom;

    //formatting
    QActionGroup* m_backgroundColorActionGroup;
    QMenu* m_backgroundColorMenu;
    QActionGroup* m_textColorActionGroup;
    QColor m_defaultDefaultTextColor;
    QMenu* m_textColorMenu;
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
    void updateCompletions();
    void completeCommandTo(const QString& completion, CommandEntry::CompletionMode mode = PreliminaryCompletion);
    void changeResultCollapsingAction();
    void showHelp();
    void toggleEnabled();

    void backgroundColorChanged(QAction*);
    void textColorChanged(QAction*);
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
