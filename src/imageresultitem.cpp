/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
    SPDX-FileCopyrightText: 2018-2022 by Alexander Semke (alexander.semke@web.de)
*/

#include "imageresultitem.h"
#include "commandentry.h"
#include "worksheetview.h"
#include "lib/imageresult.h"
#include "lib/epsresult.h"

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
    {
        Cantor::EpsResult* epsResult = static_cast<Cantor::EpsResult*>(m_result);
#ifdef WITH_EPS
        bool cacheVersionEnough = worksheet()->renderer()->scale() == 1.0 && !worksheet()->isPrinting();
        if (!epsResult->image().isNull() && cacheVersionEnough)
            setImage(epsResult->image());
        else
            setEps(m_result->data().toUrl());
#else
        setImage(epsResult->image());
#endif
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
        format = i18n("%1 files (*.%2)", imageResult->extension().toUpper(), imageResult->extension());
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
