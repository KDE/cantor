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

#ifndef COMMANDENTRY_H
#define COMMANDENTRY_H

#include "worksheetentry.h"

class Worksheet;


class CommandEntry : public WorksheetEntry
{
  Q_OBJECT
  public:
    static const QString Prompt;

    CommandEntry();
    ~CommandEntry();

    enum {Type = UserType + 2};
    int type() const;

    QString command();
    void setExpression(Cantor::Expression* expr);
    Cantor::Expression* expression();

    QString currentLine();

    bool isEmpty();

    void setContent(const QString& content);
    void setContent(const QDomElement& content, const KZip& file);

    QDomElement toXml(QDomDocument& doc, KZip* archive);
    QString toPlain(QString& commandSep, QString& commentStartingSeq, QString& commentEndingSeq);

    void showCompletion();
    void setCompletion(Cantor::CompletionObject* tc);
    void setSyntaxHelp(Cantor::SyntaxHelpObject* sh);

    void addInformation();

    bool acceptRichText();

    void removeContextHelp();
    void removeResult();

    bool evaluate(bool current);
    bool evaluateCommand();
    void interruptEvaluation();

    bool isShowingCompletionPopup();

  public slots:
    void updateEntry();
    void updatePrompt();
    void expressionChangedStatus(Cantor::Expression::Status status);
    void showAdditionalInformationPrompt(const QString& question);
    void showCompletions();
    void applySelectedCompletion();
    void completedLineChanged();
    void showSyntaxHelp();
    void completeLineTo(const QString& line, int index);

  private:
    WorksheetView* worksheetView();

  private:
    enum CompletionMode {
	PreliminaryCompletion,
	FinalCompletion
    };
  private slots:
    void invalidate();
    void resultDeleted();
    void updateCompletions();
    void completeCommandTo(const QString& completion, CompletionMode mode = PreliminaryCompletion);

  private:
    QGraphicsTextItem* m_promptItem;
    WorksheetTextItem* m_commandItem;
    WorksheetTextItem* m_resultItem;
    QGraphicsTextItem* m_errorItem;
    QList<WorksheetTextItem*> m_informationItems;
    Cantor::Expression* m_expression;
    QGraphicsLinearLayout *m_verticalLayout;

    Cantor::CompletionObject* m_completionObject;
    QPointer<KCompletionBox> m_completionBox;
    Cantor::SyntaxHelpObject* m_syntaxHelpObject;
};

#endif // COMMANDENTRY_H
