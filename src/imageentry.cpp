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
    Copyright (C) 2012 martin Kuettler <martin.kuettler@gmail.com>
 */

#include "imageentry.h"
#include "worksheetimageitem.h"

#include <KMenu>
#include <KDebug>

ImageEntry::ImageEntry(Worksheet* worksheet) : WorksheetEntry(worksheet)
{
    m_imageItem = 0;
    m_textItem = new WorksheetTextItem(this);
    m_imageWatcher = new QFileSystemWatcher(this);
    m_displaySize.width = -1;
    m_displaySize.height = -1;
    m_printSize.width = -1;
    m_printSize.height = -1;
    m_displaySize.widthUnit = ImageSize::AutoSize;
    m_displaySize.heightUnit = ImageSize::AutoSize;
    m_printSize.widthUnit = ImageSize::AutoSize;
    m_printSize.heightUnit = ImageSize::AutoSize;
    m_useDisplaySizeForPrinting = true;
    connect(m_imageWatcher, SIGNAL(fileChanged(const QString&)),
	    this, SLOT(updateEntry()));

    setFlag(QGraphicsItem::ItemIsFocusable);
    updateEntry();
}

ImageEntry::~ImageEntry()
{
}

void ImageEntry::populateMenu(KMenu *menu, const QPointF& pos)
{
    menu->addAction(i18n("Configure Image"), this, SLOT(startConfigDialog()));
    menu->addSeparator();

    WorksheetEntry::populateMenu(menu, pos);
}

bool ImageEntry::isEmpty()
{
    return false;
}

int ImageEntry::type() const
{
    return Type;
}

bool ImageEntry::acceptRichText()
{
    return false;
}

bool ImageEntry::focusEntry(int, qreal)
{
    return false;
}

void ImageEntry::setContent(const QString& content)
{
    Q_UNUSED(content);
    return;
}

void ImageEntry::setContent(const QDomElement& content, const KZip& file)
{
    Q_UNUSED(content);
    Q_UNUSED(file);
    // ...
}

QDomElement ImageEntry::toXml(QDomDocument& doc, KZip* archive)
{
    Q_UNUSED(doc);
    Q_UNUSED(archive);

    QDomElement image = doc.createElement("Image");
    // ...
    return image;
}

QString ImageEntry::toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq)
{
    Q_UNUSED(commandSep);

    return commentStartingSeq + "image: " + m_imagePath  + commentEndingSeq;
}

void ImageEntry::interruptEvaluation()
{
}

bool ImageEntry::evaluate(int evalOp)
{
    evaluateNext(evalOp);
    return true;
}

qreal ImageEntry::height()
{
    if (m_imageItem && m_imageItem->isVisible())
	return m_imageItem->height();
    else
	return m_textItem->height();
}

void ImageEntry::updateEntry()
{
    qreal oldHeight = height();
    if (m_imagePath.isEmpty()) {
	m_textItem->setPlainText(i18n("Right click here to insert image"));
	m_textItem->setVisible(true);
	if (m_imageItem)
	    m_imageItem->setVisible(false);
    }

    else {
	if (!m_imageItem)
	    m_imageItem = new WorksheetImageItem(this);

	if (m_imagePath.toLower().endsWith(".eps")) {
	    m_imageItem->setEps(m_imagePath);
	} else {
	    QImage img(m_imagePath);
	    m_imageItem->setImage(img);
	}

	if (!m_imageItem->imageIsValid()) {
	    const QString msg = i18n("Cannot load image %1").arg(m_imagePath);
	    m_textItem->setPlainText(msg);
	    m_textItem->setVisible(true);
	    m_imageItem->setVisible(false);
	} else {
	    QSizeF size;
	    if (worksheet()->isPrinting() && ! m_useDisplaySizeForPrinting)
		size = imageSize(m_printSize);
	    else
		size = imageSize(m_displaySize);
	    // Hack: Eps images need to be scaled
	    if (m_imagePath.toLower().endsWith(".eps"))
		size /= worksheet()->epsRenderer()->scale();
	    m_imageItem->setSize(size);
	    kDebug() << size;
	    m_textItem->setVisible(false);
	    m_imageItem->setVisible(true);
	}
    }

    kDebug() << oldHeight << height();
    if (oldHeight != height())
	recalculateSize();
}

QSizeF ImageEntry::imageSize(const ImageSize& imgSize)
{
    const QSize& srcSize = m_imageItem->imageSize();
    qreal w, h;
    if (imgSize.heightUnit == ImageSize::PercentSize)
	h = srcSize.height() * imgSize.height / 100;
    else if (imgSize.heightUnit == ImageSize::PixelSize)
	h = imgSize.height;
    if (imgSize.widthUnit == ImageSize::PercentSize)
	w = srcSize.width() * imgSize.width / 100;
    else if (imgSize.widthUnit == ImageSize::PixelSize)
	w = imgSize.width;

    if (imgSize.widthUnit == ImageSize::AutoSize) {
	if (imgSize.heightUnit == ImageSize::AutoSize)
	    return QSizeF(srcSize.width(), srcSize.height());
	else if (h == 0)
	    w = 0;
	else
	    w = h / srcSize.height() * srcSize.width();
    } else if (imgSize.heightUnit == ImageSize::AutoSize) {
	if (w == 0)
	    h = 0;
	else
	    h = w / srcSize.width() * srcSize.height();
    }

    return QSizeF(w,h);
}

void ImageEntry::startConfigDialog()
{
    ImageSettingsDialog* dialog = new ImageSettingsDialog(worksheet()->worksheetView());
    dialog->setData(m_imagePath, m_displaySize, m_printSize,
		    m_useDisplaySizeForPrinting);
    connect(dialog, SIGNAL(dataChanged(const QString&, const ImageSize&,
				       const ImageSize&, bool)),
	    this, SLOT(setImageData(const QString&, const ImageSize&,
				    const ImageSize&, bool)));
    dialog->show();
}

void ImageEntry::setImageData(const QString& path,
			      const ImageSize& displaySize,
			      const ImageSize& printSize,
			      bool useDisplaySizeForPrinting)
{
    if (path != m_imagePath) {
	m_imageWatcher->removePath(m_imagePath);
	m_imageWatcher->addPath(path);
	m_imagePath = path;
    }

    m_displaySize = displaySize;
    m_printSize = printSize;
    m_useDisplaySizeForPrinting = useDisplaySizeForPrinting;

    updateEntry();
}

void ImageEntry::layOutForWidth(double w, bool force)
{
    if (size().width() == w && !force)
	return;

    if (m_imageItem && m_imageItem->isVisible())
	m_imageItem->setPos(w/2 - m_imageItem->width()/2, 0);
    else
	m_textItem->setPos(w/2 - m_textItem->width()/2, 0);
    setSize(QSizeF(w, height() + VerticalMargin));
}

bool ImageEntry::wantToEvaluate()
{
    return false;
}

#include "imageentry.moc"
