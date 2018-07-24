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
    PageBreakEntry(Worksheet* worksheet);
    ~PageBreakEntry() override;

    enum {Type = UserType + 3};
    int type() const Q_DECL_OVERRIDE;

    bool isEmpty() Q_DECL_OVERRIDE;
    bool acceptRichText() Q_DECL_OVERRIDE;
    void setContent(const QString& content) Q_DECL_OVERRIDE;
    void setContent(const QDomElement& content, const KZip& file) Q_DECL_OVERRIDE;
    QDomElement toXml(QDomDocument& doc, KZip* archive) Q_DECL_OVERRIDE;
    QString toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq) Q_DECL_OVERRIDE;

    void interruptEvaluation() Q_DECL_OVERRIDE;

    void layOutForWidth(qreal w, bool force = false) Q_DECL_OVERRIDE;

    //void paint(QPainter* painter, const QStyleOptionGraphicsItem * option,
    //         QWidget * widget = 0);

  public Q_SLOTS:
    bool evaluate(WorksheetEntry::EvaluationOption evalOp = FocusNext) Q_DECL_OVERRIDE;
    void updateEntry() Q_DECL_OVERRIDE;
    void populateMenu(QMenu* menu, QPointF pos) Q_DECL_OVERRIDE;

  protected:
    bool wantToEvaluate() Q_DECL_OVERRIDE;
    bool wantFocus() Q_DECL_OVERRIDE;

  private:
    WorksheetTextItem* m_msgItem;
};

#endif /* PAGEBREAKENTRY_H */

