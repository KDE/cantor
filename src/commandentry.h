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
 */

#ifndef COMMANDENTRY_H
#define COMMANDENTRY_H

#include <QPointer>
#include <KCompletionBox>

#include "worksheetentry.h"
#include "lib/expression.h"

class Worksheet;
class ResultItem;

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

    CommandEntry(Worksheet* worksheet);
    ~CommandEntry() override;

    enum {Type = UserType + 2};
    int type() const Q_DECL_OVERRIDE;

    QString command();
    void setExpression(Cantor::Expression* expr);
    Cantor::Expression* expression();

    QString currentLine();

    bool isEmpty() Q_DECL_OVERRIDE;

    void setContent(const QString& content) Q_DECL_OVERRIDE;
    void setContent(const QDomElement& content, const KZip& file) Q_DECL_OVERRIDE;

    QDomElement toXml(QDomDocument& doc, KZip* archive) Q_DECL_OVERRIDE;
    QString toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq) Q_DECL_OVERRIDE;

    void setCompletion(Cantor::CompletionObject* tc);
    void setSyntaxHelp(Cantor::SyntaxHelpObject* sh);

    bool acceptRichText() Q_DECL_OVERRIDE;

    void removeContextHelp();

    void interruptEvaluation() Q_DECL_OVERRIDE;
    bool isShowingCompletionPopup();

    bool focusEntry(int pos = WorksheetTextItem::TopLeft, qreal xCoord = 0) Q_DECL_OVERRIDE;

    void layOutForWidth(qreal w, bool force = false) Q_DECL_OVERRIDE;

    WorksheetTextItem* highlightItem() Q_DECL_OVERRIDE;

    WorksheetCursor search(QString pattern, unsigned flags,
                           QTextDocument::FindFlags qt_flags,
                           const WorksheetCursor& pos = WorksheetCursor()) Q_DECL_OVERRIDE;

  public Q_SLOTS:
    bool evaluateCurrentItem() Q_DECL_OVERRIDE;
    bool evaluate(WorksheetEntry::EvaluationOption evalOp = FocusNext) Q_DECL_OVERRIDE;
    void addInformation();
    void removeResult();

    void showCompletion() Q_DECL_OVERRIDE;
    void selectPreviousCompletion();
    void updateEntry() Q_DECL_OVERRIDE;
    void updatePrompt();
    void expressionChangedStatus(Cantor::Expression::Status status);
    void showAdditionalInformationPrompt(const QString& question);
    void showCompletions();
    void applySelectedCompletion();
    void completedLineChanged();
    void showSyntaxHelp();
    void completeLineTo(const QString& line, int index);

    void startRemoving() Q_DECL_OVERRIDE;

    void moveToNextItem(int pos, qreal x);
    void moveToPreviousItem(int pos, qreal x);

    void populateMenu(QMenu *menu, const QPointF& pos) Q_DECL_OVERRIDE;

  protected:
    bool wantToEvaluate() Q_DECL_OVERRIDE;

  private:
    WorksheetTextItem* currentInformationItem();
    bool informationItemHasFocus();
    bool focusWithinThisItem();
    QPoint getPopupPosition();

    QPoint toGlobalPosition(const QPointF& localPos);

  private:
    enum CompletionMode {
        PreliminaryCompletion,
        FinalCompletion
    };
  private Q_SLOTS:
    void invalidate();
    void resultDeleted();
    void updateCompletions();
    void completeCommandTo(const QString& completion, CommandEntry::CompletionMode mode = PreliminaryCompletion);

  private:
    static const double HorizontalSpacing;
    static const double VerticalSpacing;

    WorksheetTextItem* m_promptItem;
    WorksheetTextItem* m_commandItem;
    ResultItem* m_resultItem;
    WorksheetTextItem* m_errorItem;
    QList<WorksheetTextItem*> m_informationItems;
    Cantor::Expression* m_expression;

    Cantor::CompletionObject* m_completionObject;
    QPointer<KCompletionBox> m_completionBox;
    Cantor::SyntaxHelpObject* m_syntaxHelpObject;

    EvaluationOption m_evaluationOption;
};

#endif // COMMANDENTRY_H
