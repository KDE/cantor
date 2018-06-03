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
    ~PlaceHolderEntry() override;

    enum {Type = UserType + 6};
    int type() const Q_DECL_OVERRIDE;

    bool isEmpty() Q_DECL_OVERRIDE;
    bool acceptRichText() Q_DECL_OVERRIDE;
    void setContent(const QString&) Q_DECL_OVERRIDE;
    void setContent(const QDomElement&, const KZip&) Q_DECL_OVERRIDE;
    QDomElement toXml(QDomDocument&, KZip*) Q_DECL_OVERRIDE;
    QString toPlain(const QString&, const QString&, const QString&) Q_DECL_OVERRIDE;
    void interruptEvaluation() Q_DECL_OVERRIDE;

    void layOutForWidth(qreal w, bool force = false) Q_DECL_OVERRIDE;

  public Q_SLOTS:
    bool evaluate(WorksheetEntry::EvaluationOption evalOp = FocusNext) Q_DECL_OVERRIDE;
    void updateEntry() Q_DECL_OVERRIDE;

    void changeSize(QSizeF s);

  protected:
    bool wantToEvaluate() Q_DECL_OVERRIDE;
};

#endif //PLACEHOLDERENTRY_H
