/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
    SPDX-FileCopyrightText: 2018-2022 by Alexander Semke (alexander.semke@web.de)
*/

#include "imageresultitem.h"
#include "commandentry.h"
#include "worksheetview.h"
#include "lib/imageresult.h"

#include <config-cantor.h>

#include <KLocalizedString>
#include <QFileDialog>
#include <QImageReader>

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

void ImageResultItem::populateMenu(QMenu* menu, QPointF)
{
    ResultItem::addCommonActions(this, menu);
}

void ImageResultItem::update()
{
    Q_ASSERT(m_result->type() == Cantor::ImageResult::Type);
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
    QString format;
    if (m_result->type() == Cantor::ImageResult::Type)
    {
        auto* imageResult = static_cast<Cantor::ImageResult*>(result());
        format = i18nc("%1 and %2 are file extensions", "%1 files (*.%2)", imageResult->extension().toUpper(), imageResult->extension());
    }
    else
        format = i18n("EPS files (*.eps)");

    const auto& fileName = QFileDialog::getSaveFileName(worksheet()->worksheetView(),
                                                           i18n("Save image result"),
                                                           /*dir*/ QString(),
                                                           format);
    if (!fileName.isEmpty())
        result()->save(fileName);
}

void ImageResultItem::deleteLater()
{
    WorksheetImageItem::deleteLater();
}
