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

#ifndef PLACEHOLDERENTRY_H
#define PLACEHOLDERENTRY_H

#include "worksheetentry.h"

class PlaceHolderEntry : public WorksheetEntry
{
  public:
    PlaceHolderEntry(Worksheet* worksheet, QSizeF s);
    ~PlaceHolderEntry() override = default;

    enum {Type = UserType + 6};
    int type() const override;

    bool isEmpty() override;
    bool acceptRichText() override;
    void setContent(const QString&) override;
    void setContent(const QDomElement&, const KZip&) override;
    void setContentFromJupyter(const QJsonObject & cell) override;
    QDomElement toXml(QDomDocument&, KZip*) override;
    QJsonValue toJupyterJson() override;
    QString toPlain(const QString&, const QString&, const QString&) override;
    void interruptEvaluation() override;

    void layOutForWidth(qreal w, bool force = false) override;

  public Q_SLOTS:
    bool evaluate(WorksheetEntry::EvaluationOption evalOp = FocusNext) override;
    void updateEntry() override;

    void changeSize(QSizeF s);

  protected:
    bool wantToEvaluate() override;
};

#endif //PLACEHOLDERENTRY_H
