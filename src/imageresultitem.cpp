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

#include "imageresultitem.h"
#include "commandentry.h"
#include "lib/imageresult.h"
#include "lib/epsresult.h"

#include <KLocalizedString>
#include <QFileDialog>
#include <QDebug>

ImageResultItem::ImageResultItem(QGraphicsObject* parent, Cantor::Result* result)
    : WorksheetImageItem(parent), ResultItem(result)
{
    update();
}

double ImageResultItem::setGeometry(double x, double y, double w)
{
    Q_UNUSED(w);
    setPos(x,y);
    return height();
}

void ImageResultItem::populateMenu(QMenu* menu, QPointF pos)
{
    ResultItem::addCommonActions(this, menu);

    menu->addSeparator();
    qDebug() << "populate Menu";
    emit menuCreated(menu, mapToParent(pos));
}

void ImageResultItem::update()
{
    Q_ASSERT(m_result->type() == Cantor::ImageResult::Type || m_result->type() == Cantor::EpsResult::Type);
    switch(m_result->type()) {
    case Cantor::ImageResult::Type:
    {
        QSize displaySize = static_cast<Cantor::ImageResult*>(m_result)->displaySize();
        if (displaySize.isValid())
            setImage(m_result->data().value<QImage>(), displaySize);
        else
            setImage(m_result->data().value<QImage>());
    }
        break;
    case Cantor::EpsResult::Type:
        setEps(m_result->data().toUrl());
        break;
    default:
        break;
    }
}

QRectF ImageResultItem::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}

double ImageResultItem::width() const
{
    return WorksheetImageItem::width();
}

double ImageResultItem::height() const
{
    return WorksheetImageItem::height();
}

void ImageResultItem::saveResult()
{
    Cantor::Result* res = result();
    const QString& filename=QFileDialog::getSaveFileName(worksheet()->worksheetView(), i18n("Save result"), QString(), res->mimeType());
    qDebug()<<"saving result to "<<filename;
    res->save(filename);
}

void ImageResultItem::deleteLater()
{
    WorksheetImageItem::deleteLater();
}

EpsRenderer* ImageResultItem::epsRenderer()
{
    return qobject_cast<Worksheet*>(scene())->epsRenderer();
}
