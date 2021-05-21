/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#include "actionbar.h"
#include "worksheet.h"
#include "worksheetentry.h"
#include "worksheettoolbutton.h"

#include <QGraphicsProxyWidget>

ActionBar::ActionBar(WorksheetEntry* parent)
    : QGraphicsObject(parent)
{
    m_pos = 0;
    m_height = 0;
    QPointF p = worksheet()->worksheetView()->viewRect().topRight();
    qreal w = qMin(parent->size().width() - WorksheetEntry::RightMargin,
                   parent->mapFromScene(p).x());
    setPos(w, 0);
    connect(worksheet()->worksheetView(), SIGNAL(viewRectChanged(QRectF)),
            this, SLOT(updatePosition()));
}

WorksheetToolButton* ActionBar::addButton(const QIcon& icon, const QString& toolTip,
                                   QObject* receiver, const char* method )
{
    WorksheetToolButton* button = new WorksheetToolButton(this);
    button->setIcon(icon);
    button->setIconScale(worksheet()->renderer()->scale());
    button->setToolTip(toolTip);
    if (receiver && method)
        connect(button, SIGNAL(clicked()), receiver, method);
    m_pos -= button->width() + 2;
    m_height = (m_height > button->height()) ? m_height : button->height();
    button->setPos(m_pos, 4);
    m_buttons.append(button);
    return button;
}

void ActionBar::addSpace()
{
    m_pos -= 8;
}

void ActionBar::updatePosition()
{
    if (!parentEntry())
        return;
    QPointF p = worksheet()->worksheetView()->viewRect().topRight();
    qreal w = qMin(parentEntry()->size().width() - WorksheetEntry::RightMargin,
                   parentEntry()->mapFromScene(p).x());
    setPos(w, 0);
    const qreal scale = worksheet()->renderer()->scale();
    foreach(WorksheetToolButton* button, m_buttons) {
        button->setIconScale(scale);
    }
}

WorksheetEntry* ActionBar::parentEntry()
{
    return qobject_cast<WorksheetEntry*>(parentObject());
}

QRectF ActionBar::boundingRect() const
{
    return QRectF(m_pos, 0, -m_pos, m_height);
}

void ActionBar::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*)
{
}

Worksheet* ActionBar::worksheet()
{
    return qobject_cast<Worksheet*>(scene());
}
