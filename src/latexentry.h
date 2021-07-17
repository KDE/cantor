/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
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

    QString plain() const;

    bool acceptRichText() override;

    bool focusEntry(int pos = WorksheetTextItem::TopLeft, qreal xCoord = 0) override;

    void setContent(const QString& content) override;
    void setContent(const QDomElement& content, const KZip& file) override;
    void setContentFromJupyter(const QJsonObject & cell) override;
    static bool isConvertableToLatexEntry(const QJsonObject& cell);

    QDomElement toXml(QDomDocument& doc, KZip* archive) override;
    QJsonValue toJupyterJson() override;
    QString toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq) override;

    void layOutForWidth(qreal entry_zone_x, qreal w, bool force = false) override;

    int searchText(const QString& text, const QString& pattern,
                   QTextDocument::FindFlags qt_flags);
    WorksheetCursor search(const QString& pattern, unsigned flags,
                           QTextDocument::FindFlags qt_flags,
                           const WorksheetCursor& pos = WorksheetCursor()) override;

  public Q_SLOTS:
    bool evaluate(WorksheetEntry::EvaluationOption evalOp = FocusNext) override;
    void resolveImagesAtCursor();
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
