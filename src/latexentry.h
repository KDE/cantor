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

#ifndef LATEXENTRY_H
#define LATEXENTRY_H

#include "worksheetentry.h"
#include "worksheettextitem.h"

class LatexEntry : public WorksheetEntry
{
  Q_OBJECT

  public:
    explicit LatexEntry(Worksheet* worksheet);
    ~LatexEntry() override = default;

    enum {Type = UserType + 5};
    int type() const override;

    bool isEmpty() override;

    bool acceptRichText() override;

    bool focusEntry(int pos = WorksheetTextItem::TopLeft, qreal xCoord = 0) override;

    void setContent(const QString& content) override;
    void setContent(const QDomElement& content, const KZip& file) override;
    void setContentFromJupyter(const QJsonObject & cell) override;
    static bool isConvertedCantorLatexEntry(const QJsonObject& cell);

    QDomElement toXml(QDomDocument& doc, KZip* archive) override;
    QJsonValue toJupyterJson() override;
    QString toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq) override;

    void interruptEvaluation() override;

    void layOutForWidth(qreal w, bool force = false) override;

    int searchText(const QString& text, const QString& pattern,
                   QTextDocument::FindFlags qt_flags);
    WorksheetCursor search(const QString& pattern, unsigned flags,
                           QTextDocument::FindFlags qt_flags,
                           const WorksheetCursor& pos = WorksheetCursor()) override;

  public Q_SLOTS:
    bool evaluate(WorksheetEntry::EvaluationOption evalOp = FocusNext) override;
    void updateEntry() override;
    void populateMenu(QMenu* menu, QPointF pos) override;

  protected:
    bool wantToEvaluate() override;
    bool eventFilter(QObject* object, QEvent* event) override;

  private:
    QString latexCode();
    bool renderLatexCode();
    bool isOneImageOnly();

  private:
    WorksheetTextItem* m_textItem;
    QTextImageFormat m_renderedFormat;
    QString m_latex;
};

#endif // LATEXENTRY_H
