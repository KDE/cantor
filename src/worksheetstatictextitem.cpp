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

#include "worksheetstatictextitem.h"

#include <QTextDocument>
#include "kdebug.h"

WorksheetStaticTextItem::WorksheetStaticTextItem(QGraphicsWidget* parent, QGraphicsLayoutItem* lparent) :
    QGraphicsTextItem(parent), QGraphicsLayoutItem(lparent ? lparent : parent)
{
    setTextInteractionFlags(Qt::TextSelectableByMouse);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

WorksheetStaticTextItem::~WorksheetStaticTextItem()
{
}

void WorksheetStaticTextItem::setGeometry(const QRectF& rect)
{
    //setTextWidth(rect.width());
    setPos(rect.topLeft());
    kDebug() << rect;
}

QSizeF WorksheetStaticTextItem::sizeHint(Qt::SizeHint which, const QSizeF & constraint ) const
{
    qreal oldTextWidth = document()->textWidth();
    QSizeF size;

    /* I am ignoring a potential height constraint here, because I do not
     * expect any. The WorksheetEntries are not constrained in their height,
     * and their child items should not be either.
     */

    switch (which) {
    case Qt::MinimumSize:
    case Qt::PreferredSize:
	document()->setTextWidth(-1);

	if (constraint.width() >= 0 && document()->textWidth() > constraint.width())
	    document()->setTextWidth(constraint.width());
	size =  document()->size();
	if (oldTextWidth >= 0)
	    document()->setTextWidth(oldTextWidth);
	return size;
    case Qt::MaximumSize:
	return QSizeF();
    default: // I don't think any other values are possible here
	return QSizeF();
    }
}

#include "worksheetstatictextitem.moc"

