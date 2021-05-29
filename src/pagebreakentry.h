/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#ifndef PAGEBREAKENTRY_H
#define PAGEBREAKENTRY_H

#include "worksheetentry.h"

class WorksheetTextItem;

class PageBreakEntry : public WorksheetEntry
{
  Q_OBJECT

  public:
    explicit PageBreakEntry(Worksheet* worksheet);
    ~PageBreakEntry() override = default;

    enum {Type = UserType + 3};
    int type() const override;

    bool isEmpty() override;
    bool acceptRichText() override;
    void setContent(const QString& content) override;
    void setContent(const QDomElement& content, const KZip& file) override;
    void setContentFromJupyter(const QJsonObject & cell) override;
    static bool isConvertableToPageBreakEntry(const QJsonObject& cell);
    QDomElement toXml(QDomDocument& doc, KZip* archive) override;
    QJsonValue toJupyterJson() override;
    QString toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq) override;

    void layOutForWidth(qreal entry_zone_x, qreal w, bool force = false) override;

    //void paint(QPainter* painter, const QStyleOptionGraphicsItem * option,
    //         QWidget * widget = 0);

  public Q_SLOTS:
    bool evaluate(WorksheetEntry::EvaluationOption evalOp = FocusNext) override;
    void updateEntry() override;
    void populateMenu(QMenu* menu, QPointF pos) override;

  protected:
    bool wantToEvaluate() override;
    bool wantFocus() override;

  private:
    WorksheetTextItem* m_msgItem;
};

#endif /* PAGEBREAKENTRY_H */

