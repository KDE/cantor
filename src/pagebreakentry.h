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

    void interruptEvaluation() override;

    void layOutForWidth(qreal w, bool force = false) override;

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

