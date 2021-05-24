/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#include "placeholderentry.h"

#include <QPropertyAnimation>
#include <QJsonObject>

PlaceHolderEntry::PlaceHolderEntry(Worksheet* worksheet, QSizeF s)
    : WorksheetEntry(worksheet)
{
    m_controlElement.hide();
    setSize(s);
}

int PlaceHolderEntry::type() const
{
    return Type;
}

bool PlaceHolderEntry::isEmpty()
{
    /*
    // This is counter-intuitive. isEmpty() is used to find out whether a new
    // CommandEntry needs to be appended, and a PlaceHolderEntry should never
    // prevent that.
    return false;
    */
    return true;
}

bool PlaceHolderEntry::acceptRichText()
{
    return false;
}

void PlaceHolderEntry::setContent(const QString&)
{
}

void PlaceHolderEntry::setContent(const QDomElement&, const KZip&)
{
}

void PlaceHolderEntry::setContentFromJupyter(const QJsonObject& cell)
{
    Q_UNUSED(cell);
    return;
}

QJsonValue PlaceHolderEntry::toJupyterJson()
{
    return QJsonValue();
}


QDomElement PlaceHolderEntry::toXml(QDomDocument&, KZip*)
{
    return QDomElement();
}

QString PlaceHolderEntry::toPlain(const QString&, const QString&, const QString&){
    return QString();
}

void PlaceHolderEntry::layOutForWidth(qreal entry_zone_x, qreal w, bool force)
{
    Q_UNUSED(entry_zone_x);
    Q_UNUSED(w);
    Q_UNUSED(force);
}

bool PlaceHolderEntry::evaluate(EvaluationOption evalOp)
{
    evaluateNext(evalOp);
    return true;
}

void PlaceHolderEntry::updateEntry()
{
}

bool PlaceHolderEntry::wantToEvaluate()
{
    return false;
}

void PlaceHolderEntry::changeSize(QSizeF s)
{
    if (!worksheet()->animationsEnabled()) {
        setSize(s);
        worksheet()->updateEntrySize(this);
        return;
    }
    if (aboutToBeRemoved())
        return;

    if (animationActive())
        endAnimation();

    QPropertyAnimation* sizeAn = sizeChangeAnimation(s);

    sizeAn->setEasingCurve(QEasingCurve::InOutQuad);
    sizeAn->start(QAbstractAnimation::DeleteWhenStopped);
}

