/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#ifndef RESULTITEM_H
#define RESULTITEM_H

/*
 * This is a common superclass of all result items. Unfortunately this class
 * cannot inherit QGraphicsItem or QObject, because the subclasses inherit
 * these from an other source (TextResultItem inherits WorksheetTextItem, for
 * example). Therefore this class mainly offers the interface, and the
 * implementations are done in each subclasses, even when the code is literally
 * the same for them.
 */

namespace Cantor {
    class Result;
}

class CommandEntry;
class WorksheetEntry;

class QMenu;
class QObject;
class QPointF;
class QGraphicsObject;

class ResultItem
{
  public:
    ResultItem(Cantor::Result* result);
    virtual ~ResultItem() = default;

    static ResultItem* create(WorksheetEntry* parent, Cantor::Result* result);

    virtual double setGeometry(double x, double y, double w) = 0;
    virtual void populateMenu(QMenu* menu, QPointF pos) = 0;

    virtual void update() = 0;

    virtual void deleteLater() = 0;

    virtual double width() const = 0;
    virtual double height() const = 0;

    QGraphicsObject* graphicsObject();
    Cantor::Result* result();
    CommandEntry* parentEntry();

  protected:
    void addCommonActions(QObject* self, QMenu* menu);
    void needRemove();

  protected:
    Cantor::Result* m_result;
};

#endif // RESULTITEM_H
