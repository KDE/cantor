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
    Copyright (C) 2011 martin Kuettler <martin.kuettler@gmail.com>
 */

#include "imageentry.h"
#include "worksheet.h"

#ifdef LIBSPECTRE_FOUND
  #include "libspectre/spectre.h"
#endif

#include <kcolorscheme.h>
#include <kmenu.h>
#include <kicon.h>
#include <klocale.h>
#include <kdebug.h>

#include <kdebug.h>
#include <qurl.h>

ImageEntry::ImageEntry(QTextCursor position, Worksheet* parent) :
    WorksheetEntry( position, parent )
{
    m_imagePath = QString();
    m_displaySize.width = -1;
    m_displaySize.height = -1;
    m_printSize.width = -1;
    m_printSize.height = -1;
    m_displaySize.widthUnit = QString();
    m_displaySize.heightUnit = QString();
    m_printSize.widthUnit = QString();
    m_printSize.heightUnit = QString();
    m_useDisplaySizeForPrinting = true;
    m_settingsDialog = 0;

    connect(&m_imageWatcher, SIGNAL(fileChanged(const QString&)), this, SLOT(update()));

    /*QTextFrameFormat frameFormat = m_frame->frameFormat();
      frameFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);
      m_frame->setFrameFormat(frameFormat);*/

    update();
}

ImageEntry::~ImageEntry()
{
    if (m_settingsDialog != 0)
	delete m_settingsDialog;
}

int ImageEntry::type()
{
    return Type;
}

bool ImageEntry::isEmpty()
{
    /* Are we empty? */
    return m_imagePath.isEmpty();
}

QTextCursor ImageEntry::closestValidCursor(const QTextCursor& cursor)
{
    Q_UNUSED(cursor);
    return firstValidCursorPosition();
}

QTextCursor ImageEntry::firstValidCursorPosition()
{
    return m_frame->lastCursorPosition();
}

QTextCursor ImageEntry::lastValidCursorPosition()
{
    return m_frame->lastCursorPosition();
}

bool ImageEntry::isValidCursor(const QTextCursor& cursor)
{
    Q_UNUSED(cursor);
    return false;
}

bool ImageEntry::worksheetContextMenuEvent(QContextMenuEvent* event, const QTextCursor& cursor)
{
    Q_UNUSED(cursor);
    KMenu* defaultMenu = new KMenu(m_worksheet);

    defaultMenu->addAction(i18n("Configure Image"), this, SLOT(startConfigDialog()));
    defaultMenu->addSeparator();
    
    if(!m_worksheet->isRunning())
	defaultMenu->addAction(KIcon("system-run"),i18n("Evaluate Worksheet"),m_worksheet,SLOT(evaluate()),0);
    else
	defaultMenu->addAction(KIcon("process-stop"),i18n("Interrupt"),m_worksheet,SLOT(interrupt()),0);
    defaultMenu->addSeparator();

    defaultMenu->addAction(KIcon("edit-delete"),i18n("Remove Entry"), m_worksheet, SLOT(removeCurrentEntry()));
    
    createSubMenuInsert(defaultMenu);

    defaultMenu->popup(event->globalPos());
    
    return true;
    
}

bool ImageEntry::acceptRichText()
{
    return false;
}

bool ImageEntry::acceptsDrop(const QTextCursor& cursor)
{
    // Maybe we will, someday.
    Q_UNUSED(cursor);
    return false;
}

void ImageEntry::setContent(const QString& content)
{
    Q_UNUSED(content);
    return;
}

void ImageEntry::setContent(const QDomElement& content, const KZip& file)
{
    Q_UNUSED(file);

    QDomElement pathElement = content.firstChildElement("Path");
    QDomElement displayElement = content.firstChildElement("Display");
    QDomElement printElement = content.firstChildElement("Print");
    m_imagePath = pathElement.text();
    m_displaySize.width = displayElement.attribute("width").toDouble();
    m_displaySize.height = displayElement.attribute("height").toDouble();
    m_displaySize.widthUnit = displayElement.attribute("widthUnit");
    m_displaySize.heightUnit = displayElement.attribute("heightUnit");
    m_useDisplaySizeForPrinting = printElement.attribute("useDisplaySize").toInt();
    m_printSize.width = printElement.attribute("width").toDouble();
    m_printSize.height = printElement.attribute("height").toDouble();
    m_printSize.widthUnit = printElement.attribute("widthUnit");
    m_printSize.heightUnit = printElement.attribute("heightUnit");

    update();
}

QDomElement ImageEntry::toXml(QDomDocument& doc, KZip* archive)
{
    Q_UNUSED(archive);
    
    QDomElement image = doc.createElement("Image");
    QDomElement path = doc.createElement("Path");
    QDomText pathText = doc.createTextNode(m_imagePath);
    path.appendChild(pathText);
    image.appendChild(path);
    QDomElement display = doc.createElement("Display");
    display.setAttribute("width", m_displaySize.width);
    display.setAttribute("widthUnit", m_displaySize.widthUnit);
    display.setAttribute("height", m_displaySize.height);
    display.setAttribute("heightUnit", m_displaySize.heightUnit);
    image.appendChild(display);
    QDomElement print = doc.createElement("Print");
    print.setAttribute("useDisplaySize", m_useDisplaySizeForPrinting);
    print.setAttribute("width", m_printSize.width);
    print.setAttribute("widthUnit", m_printSize.widthUnit);
    print.setAttribute("height", m_printSize.height);
    print.setAttribute("heightUnit", m_printSize.heightUnit);
    image.appendChild(print);
    /* This element contains redundant information, but constructing
     * the size string with xslt seems to be an unelegant solution to me.
     */
    QDomElement latexSize = doc.createElement("LatexSizeString");
    QString sizeString;
    if (m_useDisplaySizeForPrinting) {
	sizeString = makeLatexSizeString(m_displaySize);
    }
    else {
	sizeString = makeLatexSizeString(m_printSize);
    }
    QDomText latexSizeString = doc.createTextNode(sizeString);
    latexSize.appendChild(latexSizeString);
    image.appendChild(latexSize);
    
    return image;
}

QString ImageEntry::toPlain(QString& commandSep, QString& commentStartingSeq, QString& commentEndingSeq)
{
    Q_UNUSED(commandSep);
    
    return commentStartingSeq + "image: " + m_imagePath  + commentEndingSeq;
    
}

void ImageEntry::interruptEvaluation()
{
    return;
}

bool ImageEntry::evaluate(bool current)
{
    Q_UNUSED(current);
    return true;
}

bool ImageEntry::worksheetKeyPressEvent(QKeyEvent* event, const QTextCursor& cursor)
{
    if (WorksheetEntry::worksheetKeyPressEvent(event, cursor))
    {
        return true;
    }

    return true;
}

void ImageEntry::update()
{
    QTextCursor cursor(m_frame->firstCursorPosition());
    cursor.setPosition(m_frame->lastPosition(), QTextCursor::KeepAnchor);
    cursor.removeSelectedText();    

    if (m_imagePath.isEmpty())
    {
	if (m_worksheet->isPrinting())
	{
	    QTextFrameFormat frameFormat = m_frame->frameFormat();
	    frameFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);

	    m_frame->setFrameFormat(frameFormat);
	    return;
	}
	QTextBlockFormat block(cursor.blockFormat());
	block.setAlignment(Qt::AlignCenter);
	cursor.setBlockFormat(block);

	KColorScheme color = KColorScheme(QPalette::Normal, KColorScheme::View);
	cursor.insertText(i18n("Right click here to insert image"));

	return;
    }

    QImage img(m_imagePath);

    if (img.isNull())
    {
	if (m_worksheet->isPrinting())
	{
	    QTextFrameFormat frameFormat = m_frame->frameFormat();
	    frameFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);

	    m_frame->setFrameFormat(frameFormat);
	    return;
	}
	QTextBlockFormat block(cursor.blockFormat());
	block.setAlignment(Qt::AlignCenter);
	cursor.setBlockFormat(block);

	KColorScheme color = KColorScheme(QPalette::Normal, KColorScheme::View);
	cursor.insertText(i18n("Cannot load image ")+m_imagePath);

	return;
    }

    QTextImageFormat imgFormat;

#ifdef LIBSPECTRE_FOUND
    // a better way to test for eps-files?
    if (m_imagePath.endsWith(".eps"))
    {
      imgFormat = renderEps(m_printSize);
    }
    else
#endif
    if (m_worksheet->isPrinting() && !m_useDisplaySizeForPrinting)
    {
        double w, h;
	m_worksheet->document()->addResource(QTextDocument::ImageResource, QUrl(m_imagePath), QVariant(img));
	imgFormat.setName(m_imagePath);
	calculateImageSize(img.width(), img.height(), m_printSize, w, h);
	imgFormat.setWidth(w);
	imgFormat.setHeight(h);
    }
    else
    {
        double w, h;
	m_worksheet->document()->addResource(QTextDocument::ImageResource, QUrl(m_imagePath), QVariant(img));
	imgFormat.setName(m_imagePath);
	calculateImageSize(img.width(), img.height(), m_displaySize, w, h);
	imgFormat.setWidth(w);
	imgFormat.setHeight(h);
    }

    QTextBlockFormat block(cursor.blockFormat());
    block.setAlignment(Qt::AlignCenter);
    cursor.setBlockFormat(block);
    cursor.insertImage(imgFormat);
}

void ImageEntry::setImageData(const QString& path, const ImageSize& displaySize, const ImageSize& printSize, bool useDisplaySizeForPrinting)
{
    m_imageWatcher.removePath(m_imagePath);
    m_imageWatcher.addPath(path);

    m_imagePath = path;
    m_displaySize = displaySize;
    m_printSize = printSize;
    m_useDisplaySizeForPrinting = useDisplaySizeForPrinting;

    update();
}

void ImageEntry::startConfigDialog()
{
    if (m_settingsDialog == 0)
    {
	m_settingsDialog = new ImageSettingsDialog(m_worksheet);
	m_settingsDialog->setData(m_imagePath, m_displaySize, m_printSize, m_useDisplaySizeForPrinting);
	connect(m_settingsDialog, SIGNAL(dataChanged(const QString&, const ImageSize&, const ImageSize&, bool)), 
		this, SLOT(setImageData(const QString&, const ImageSize&, const ImageSize&, bool)));
    }
  
    if (m_settingsDialog->isHidden())
	m_settingsDialog->show();
    else
	m_settingsDialog->activateWindow();
}

void ImageEntry::calculateImageSize(int imgWidth, int imgHeight, const ImageSize& imageSize, double& newWidth, double& newHeight)
{
    if (imgWidth == 0 || imgHeight == 0)
    {
	newWidth = 0;
	newHeight = 0;
	return;
    }
    
    if (imageSize.heightUnit == "%")
	newHeight = imgHeight * imageSize.height / 100;
    else if (imageSize.heightUnit == "px")
	newHeight = imageSize.height;
    if (imageSize.widthUnit == "%")
	newWidth= imgWidth * imageSize.width / 100;
    else if (imageSize.widthUnit == "px")
	newWidth = imageSize.width;
    
    if (imageSize.widthUnit == "(auto)")
    {
	if (imageSize.heightUnit == "(auto)") 
	{
	    newWidth = imgWidth;
	    newHeight = imgHeight;
	    return;
	}
	else 
	    newWidth = newHeight / imgHeight * imgWidth;
    }
    else if (imageSize.heightUnit == "(auto)")
	newHeight = newWidth / imgWidth * imgHeight;
}

QString ImageEntry::makeLatexSizeString(const ImageSize& imageSize)
{
    // We use the transformation 1 px = 1/72 in ( = 1 pt in Latex)
    // TODO: Handle missing cases; that requires the image size to be known
    //  (so it can only be done when m_imagePath points to a valid image)

    QString sizeString="";
    if (imageSize.widthUnit == "(auto)" && imageSize.heightUnit == "(auto)") {
	return QString("");
    }

    if (imageSize.widthUnit == "%" && (imageSize.heightUnit == "(auto)" ||
				       (imageSize.heightUnit == "%" &&
					imageSize.width == imageSize.height))) {
	return "[scale=" + QString::number(imageSize.width / 100) + "]";
    }
    else if (imageSize.widthUnit == "(auto)" && imageSize.heightUnit == "%") {
	return "[scale=" + QString::number(imageSize.height / 100) + "]";
    }

    if (imageSize.heightUnit == "px")
	sizeString = "height=" + QString::number(imageSize.height) + "pt";
    if (imageSize.widthUnit == "px") {
	if (!sizeString.isEmpty())
	    sizeString += ",";
	sizeString += "width=" + QString::number(imageSize.width) + "pt";
    }
    return "[" + sizeString + "]";
}

#ifdef LIBSPECTRE_FOUND
QTextImageFormat ImageEntry::renderEps(const ImageSize& imageSize)
{
    QTextImageFormat epsCharFormat;

    SpectreDocument* doc=spectre_document_new();;
    SpectreRenderContext* rc=spectre_render_context_new();

    spectre_document_load(doc, m_imagePath.toUtf8());

    int w, h;
    spectre_document_get_page_size(doc, &w, &h);
    kDebug()<<"dimension: "<<w<<"x"<<h;

    double newWidth, newHeight;
    calculateImageSize(w, h, imageSize, newWidth, newHeight);

    double scale, xScale, yScale;
    if(m_worksheet->isPrinting())
        scale=4.0; //4x for high resolution
    else
        scale=1.0;
    xScale = newWidth/w * scale;
    yScale = newHeight/h * scale;

    unsigned char* data;
    int rowLength;

    spectre_render_context_set_scale(rc, xScale, yScale);
    spectre_document_render_full(doc, rc, &data, &rowLength);

    QImage img(data, w*xScale, h*yScale, rowLength, QImage::Format_RGB32);

    m_worksheet->document()->addResource(QTextDocument::ImageResource, m_imagePath, QVariant(img) );
    epsCharFormat.setName(m_imagePath);
    epsCharFormat.setWidth(newWidth);
    epsCharFormat.setHeight(newHeight);

    spectre_document_free(doc);
    spectre_render_context_free(rc);

    return epsCharFormat;
}
#endif
