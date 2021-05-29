/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#ifndef ACTIONBAR_H
#define ACTIONBAR_H

#include <QGraphicsObject>

#include <QIcon>

class Worksheet;
class WorksheetEntry;
class WorksheetToolButton;

class ActionBar : public QGraphicsObject
{
  Q_OBJECT
  public:
    explicit ActionBar(WorksheetEntry* parent);
    ~ActionBar() override = default;

    WorksheetToolButton* addButton(const QIcon& icon, const QString& toolTip,
                                   QObject* receiver = nullptr,
                                   const char* method = nullptr);
    void addSpace();

    WorksheetEntry* parentEntry();

    QRectF boundingRect() const override;
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;

  public Q_SLOTS:
    void updatePosition();

  private:
    Worksheet* worksheet();

  private:
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    QList<WorksheetToolButton*> m_buttons;
    qreal m_pos;
    qreal m_height;
};

#endif // ACTIONBAR_H
