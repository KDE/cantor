/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
    SPDX-FileCopyrightText: 2018-2022 by Alexander Semke (alexander.semke@web.de)
*/

#include "imageresultitem.h"
#include "commandentry.h"
#include "worksheetview.h"
#include "lib/imageresult.h"
#include "lib/pdfresult.h"

#include <config-cantor.h>

#include <KLocalizedString>
#include <QFileDialog>
#include <QGraphicsSceneMouseEvent>
#include <QImageReader>

ImageResultItem::ImageResultItem(QGraphicsObject* parent, Cantor::Result* result)
    : WorksheetImageItem(parent), ResultItem(result)
{
    setResizable(true);
    connect(this, &WorksheetImageItem::resizeStarted, this, &ImageResultItem::beginResizePreview);
    connect(this, &WorksheetImageItem::resizePreviewChanged, this, &ImageResultItem::updateResizePreview);
    connect(this, &WorksheetImageItem::resizeFinished, this, &ImageResultItem::applyDisplaySize, Qt::QueuedConnection);
    m_resizePreviewTimer.setInterval(16);
    m_resizePreviewTimer.setSingleShot(true);
    connect(&m_resizePreviewTimer, &QTimer::timeout, this, &ImageResultItem::flushResizePreview);
    update();

    if (m_result->type() == Cantor::ImageResult::Type) {
        auto* imageResult = static_cast<Cantor::ImageResult*>(m_result);
        m_originalSize = imageResult->originalSize();
        if (!m_originalSize.isValid()) {
            m_originalSize = size().toSize();
            imageResult->setOriginalSize(m_originalSize);
        }
    }
    else if (m_result->type() == Cantor::PdfResult::Type) {
        auto* pdfResult = static_cast<Cantor::PdfResult*>(m_result);
        m_originalSize = pdfResult->originalSize();
        if (!m_originalSize.isValid()) {
            if (pdfResult->displaySize().isValid())
                m_originalSize = pdfResult->renderToImage(1.0, false).deviceIndependentSize().toSize();
            else
                m_originalSize = size().toSize();
            pdfResult->setOriginalSize(m_originalSize);
        }
    }
}

double ImageResultItem::setGeometry(double x, double y, double w)
{
    return WorksheetImageItem::setGeometry(x, y, w);
}

void ImageResultItem::populateMenu(QMenu* menu, QPointF)
{
    menu->addAction(QIcon::fromTheme(QLatin1String("zoom-original")), i18n("Original Size"), this, &ImageResultItem::restoreOriginalSize);
    menu->addSeparator();
    ResultItem::addCommonActions(this, menu);
}

void ImageResultItem::update()
{
    Q_ASSERT(m_result->type() == Cantor::ImageResult::Type || m_result->type() == Cantor::PdfResult::Type);
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
    case Cantor::PdfResult::Type:
        renderPdf(static_cast<Cantor::PdfResult*>(m_result)->displaySize());
        break;
    default:
        break;
    }
}

void ImageResultItem::beginResizePreview(bool fromTopCorner)
{
    auto* commandEntry = parentEntry();
    if (!commandEntry)
        return;

    m_parentZValue = commandEntry->zValue();
    commandEntry->setZValue(1000.0);
    m_zValue = zValue();
    setZValue(1000.0);
    m_parentHeightBeforeResize = commandEntry->size().height();
    m_imageHeightBeforeResize = height();
    m_pendingPreviewSize = size();
    m_resizePreviewDirty = false;
    m_resizePreviewActive = true;
    m_moveFollowingEntriesDuringResize = !fromTopCorner;
}

void ImageResultItem::updateResizePreview(const QSizeF& size)
{
    if (!m_moveFollowingEntriesDuringResize)
        return;

    m_pendingPreviewSize = size;
    m_resizePreviewDirty = true;
    if (!m_resizePreviewTimer.isActive())
        m_resizePreviewTimer.start();
}

void ImageResultItem::flushResizePreview()
{
    if (!m_resizePreviewActive || !m_resizePreviewDirty)
        return;

    if (auto* commandEntry = parentEntry()) {
        const qreal height = m_parentHeightBeforeResize + m_pendingPreviewSize.height() - m_imageHeightBeforeResize;
        commandEntry->setHeightForPreview(height);
    }
    m_resizePreviewDirty = false;
}

void ImageResultItem::applyDisplaySize(const QSizeF& size)
{
    m_resizePreviewTimer.stop();
    m_resizePreviewDirty = false;

    const QSize displaySize(qRound(size.width()), qRound(size.height()));
    setSize(displaySize);
    if (m_result->type() == Cantor::ImageResult::Type) {
        auto* imageResult = static_cast<Cantor::ImageResult*>(m_result);
        imageResult->setDisplaySize(displaySize);
        setImage(imageResult->renderToDisplaySize(displaySize), displaySize);
    }
    else if (m_result->type() == Cantor::PdfResult::Type) {
        auto* pdfResult = static_cast<Cantor::PdfResult*>(m_result);
        pdfResult->setDisplaySize(displaySize);
        renderPdf(displaySize);
    }

    if (auto* commandEntry = parentEntry())
        commandEntry->recalculateSize();
    if (m_resizePreviewActive) {
        if (auto* commandEntry = parentEntry())
            commandEntry->setZValue(m_parentZValue);
        setZValue(m_zValue);
        m_resizePreviewActive = false;
    }
    if (worksheet())
        worksheet()->setModified();
}

void ImageResultItem::renderPdf(const QSize& displaySize)
{
    auto* pdfResult = static_cast<Cantor::PdfResult*>(m_result);
    if (!displaySize.isValid()) {
        setImage(pdfResult->data().value<QImage>());
        return;
    }

    QImage image = pdfResult->renderToDisplaySize(displaySize);
    if (image.isNull())
        image = pdfResult->data().value<QImage>();
    setImage(image, displaySize);
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
    else if (m_result->type() == Cantor::PdfResult::Type)
        format = i18n("PDF files (*.pdf)");
    else
        format = i18n("EPS files (*.eps)");

    const auto& fileName = QFileDialog::getSaveFileName(worksheet()->worksheetView(),
                                                           i18n("Save image result"),
                                                           /*dir*/ QString(),
                                                           format);
    if (!fileName.isEmpty())
        result()->save(fileName);
}

void ImageResultItem::restoreOriginalSize()
{
    if (m_originalSize.isValid() && m_originalSize != size().toSize())
        applyDisplaySize(m_originalSize);
}

void ImageResultItem::deleteLater()
{
    WorksheetImageItem::deleteLater();
}

void ImageResultItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (auto* commandEntry = parentEntry())
        commandEntry->resultItemClicked(m_result);

    WorksheetImageItem::mousePressEvent(event);
}
