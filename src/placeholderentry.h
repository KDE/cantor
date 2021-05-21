/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
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

    void layOutForWidth(qreal entry_zone_x, qreal w, bool force = false) override;

  public Q_SLOTS:
    bool evaluate(WorksheetEntry::EvaluationOption evalOp = FocusNext) override;
    void updateEntry() override;

    void changeSize(QSizeF s);

  protected:
    bool wantToEvaluate() override;
};

#endif //PLACEHOLDERENTRY_H
