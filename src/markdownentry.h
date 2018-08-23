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

class MarkdownEntry : public WorksheetEntry
{
  Q_OBJECT
  public:
    MarkdownEntry(Worksheet* worksheet);
    ~MarkdownEntry() override;

    enum {Type = UserType + 7};
    int type() const Q_DECL_OVERRIDE;

    bool isEmpty() Q_DECL_OVERRIDE;

    bool acceptRichText() Q_DECL_OVERRIDE;

    bool focusEntry(int pos = WorksheetTextItem::TopLeft, qreal xCoord=0) Q_DECL_OVERRIDE;

    void setContent(const QString& content) Q_DECL_OVERRIDE;
    void setContent(const QDomElement& content, const KZip& file) Q_DECL_OVERRIDE;

    QDomElement toXml(QDomDocument& doc, KZip* archive) Q_DECL_OVERRIDE;
    QString toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq) Q_DECL_OVERRIDE;

    void interruptEvaluation() Q_DECL_OVERRIDE;

    void layOutForWidth(qreal w, bool force = false) Q_DECL_OVERRIDE;

    WorksheetCursor search(const QString& pattern, unsigned flags,
                           QTextDocument::FindFlags qt_flags,
                           const WorksheetCursor& pos = WorksheetCursor()) Q_DECL_OVERRIDE;

  public Q_SLOTS:
    bool evaluate(WorksheetEntry::EvaluationOption evalOp = FocusNext) Q_DECL_OVERRIDE;
    void updateEntry() Q_DECL_OVERRIDE;

  protected:
    bool renderMarkdown(QString& plain);
    bool eventFilter(QObject* object, QEvent* event) Q_DECL_OVERRIDE;
    bool wantToEvaluate() Q_DECL_OVERRIDE;

  protected:
    WorksheetTextItem* m_textItem;
    QString plain;
    QString html;
    bool rendered;
};

#endif //MARKDOWNENTRY_H
