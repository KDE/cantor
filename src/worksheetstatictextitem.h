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

#ifndef WORKSHEET_STATIC_TEXT_ENTRY_H
#define WORKSHEET_STATIC_TEXT_ENTRY_H

#include <QGraphicsTextItem>
#include <QGraphicsLayoutItem>
#include <QGraphicsWidget>

#include <KMenu>

/*
 * A QGraphicsTextItem that is also a QGraphicsLayoutItem, so that it can
 * be used in a QGraphicsLayout. This class is intended for static text,
 * WorksheetTextItem is a subclass for editable text.
 */

class WorksheetStaticTextItem 
    : public QGraphicsTextItem, public QGraphicsLayoutItem
{
  Q_OBJECT
  Q_INTERFACES(QGraphicsLayoutItem)
  public:
    WorksheetStaticTextItem(QGraphicsWidget* parent, QGraphicsLayoutItem* lparent = 0);
    virtual ~WorksheetStaticTextItem();

    void setGeometry(const QRectF& rect);

    virtual void populateMenu(KMenu *menu);

  public slots:
    void copy();

  protected:
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint = QSizeF() ) const;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
};

#endif //WORKSHEET_STATIC_TEXT_ENTRY_H
