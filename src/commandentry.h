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
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
    Copyright (C) 2012 Martin Kuettler <martin.kuettler@gmail.com>
    Copyright (C) 2018 Alexander Semke <alexander.semke@web.de>
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

    explicit CommandEntry(Worksheet* worksheet);
    ~CommandEntry() override;

    enum {Type = UserType + 2};
    int type() const override;

    QString command();
    void setExpression(Cantor::Expression* expr);
    Cantor::Expression* expression();

    QString currentLine();

    bool isEmpty() override;

    void setContent(const QString& content) override;
    void setContent(const QDomElement& content, const KZip& file) override;
    void setContentFromJupyter(const QJsonObject& cell) override;

    QDomElement toXml(QDomDocument& doc, KZip* archive) override;
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

    void showCompletion() override;
    void selectPreviousCompletion();
    void updateEntry() override;
    void updatePrompt(const QString& postfix = CommandEntry::Prompt);
    void expressionChangedStatus(Cantor::Expression::Status status);
    void showAdditionalInformationPrompt(const QString& question);
    void showCompletions();
    void applySelectedCompletion();
    void completedLineChanged();
    void showSyntaxHelp();
    void completeLineTo(const QString& line, int index);

    void startRemoving() override;

    void moveToNextItem(int pos, qreal x);
    void moveToPreviousItem(int pos, qreal x);

    void populateMenu(QMenu* menu, QPointF pos) override;

  protected:
    bool wantToEvaluate() override;

  private:
    WorksheetTextItem* currentInformationItem();
    bool informationItemHasFocus();
    bool focusWithinThisItem();
    QPoint getPopupPosition();
    QPoint toGlobalPosition(QPointF localPos);
    void initMenus();

  private:
    enum CompletionMode {
        PreliminaryCompletion,
        FinalCompletion
    };
  private Q_SLOTS:
    void invalidate();
    void resultDeleted();
    void clearResultItems();
    void removeResultItem(int index);
    void replaceResultItem(int index);
    void updateCompletions();
    void completeCommandTo(const QString& completion, CommandEntry::CompletionMode mode = PreliminaryCompletion);
    void changeResultCollapsingAction();

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

  private:
    static const double HorizontalSpacing;
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
};

#endif // COMMANDENTRY_H
