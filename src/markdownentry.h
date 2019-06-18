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
    Copyright (C) 2018 Yifei Wu <kqwyfg@gmail.com>
 */

#ifndef MARKDOWNENTRY_H
#define MARKDOWNENTRY_H

#include "worksheetentry.h"
#include "worksheettextitem.h"

class QJsonObject;

class MarkdownEntry : public WorksheetEntry
{
  Q_OBJECT
  public:
    explicit MarkdownEntry(Worksheet* worksheet);
    ~MarkdownEntry() override = default;

    enum {Type = UserType + 7};
    int type() const override;

    bool isEmpty() override;

    bool acceptRichText() override;

    bool focusEntry(int pos = WorksheetTextItem::TopLeft, qreal xCoord=0) override;

    void setContent(const QString& content) override;
    void setContent(const QDomElement& content, const KZip& file) override;
    void setContentFromJupyter(const QJsonObject& cell) override;

    QDomElement toXml(QDomDocument& doc, KZip* archive) override;
    QJsonValue toJupyterJson() override;
    QString toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq) override;

    void interruptEvaluation() override;

    void layOutForWidth(qreal w, bool force = false) override;

    WorksheetCursor search(const QString& pattern, unsigned flags,
                           QTextDocument::FindFlags qt_flags,
                           const WorksheetCursor& pos = WorksheetCursor()) override;

  public Q_SLOTS:
    bool evaluate(WorksheetEntry::EvaluationOption evalOp = FocusNext) override;
    void updateEntry() override;

  protected:
    bool renderMarkdown(QString& plain);
    bool eventFilter(QObject* object, QEvent* event) override;
    bool wantToEvaluate() override;
    void setRenderedHtml(const QString& html);
    void setPlainText(const QString& plain);
    QTextCursor findLatexCode(const QTextCursor& cursor = QTextCursor()) const;
    QString adaptJupyterMarkdown(const QString& markdown);
    QString convert(const QString& markdown, QString& tail);

  protected:
    WorksheetTextItem* m_textItem;
    QString plain;
    QString html;
    bool rendered;
};

#endif //MARKDOWNENTRY_H
