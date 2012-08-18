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
#include "actionbar.h"

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
    m_displaySize.widthUnit = ImageSize::Auto;
    m_displaySize.heightUnit = ImageSize::Auto;
    m_printSize.widthUnit = ImageSize::Auto;
    m_printSize.heightUnit = ImageSize::Auto;
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
    menu->addAction(KIcon("configure"), i18n("Configure Image"),
                    this, SLOT(startConfigDialog()));
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

void ImageEntry::setContent(const QString& content)
{
    Q_UNUSED(content);
    return;
}

void ImageEntry::setContent(const QDomElement& content, const KZip& file)
{
    Q_UNUSED(file);
    static QStringList unitNames;
    if (unitNames.isEmpty())
        unitNames << "(auto)" << "px" << "%";

    QDomElement pathElement = content.firstChildElement("Path");
    QDomElement displayElement = content.firstChildElement("Display");
    QDomElement printElement = content.firstChildElement("Print");
    m_imagePath = pathElement.text();
    m_displaySize.width = displayElement.attribute("width").toDouble();
    m_displaySize.height = displayElement.attribute("height").toDouble();
    m_displaySize.widthUnit = unitNames.indexOf(displayElement.attribute("widthUnit"));
    m_displaySize.heightUnit = unitNames.indexOf(displayElement.attribute("heightUnit"));
    m_useDisplaySizeForPrinting = printElement.attribute("useDisplaySize").toInt();
    m_printSize.width = printElement.attribute("width").toDouble();
    m_printSize.height = printElement.attribute("height").toDouble();
    m_printSize.widthUnit = unitNames.indexOf(printElement.attribute("widthUnit"));
    m_printSize.heightUnit = unitNames.indexOf(printElement.attribute("heightUnit"));
    updateEntry();
}

QDomElement ImageEntry::toXml(QDomDocument& doc, KZip* archive)
{
    Q_UNUSED(archive);

    static QStringList unitNames;
    if (unitNames.isEmpty())
        unitNames << "(auto)" << "px" << "%";

    QDomElement image = doc.createElement("Image");
    QDomElement path = doc.createElement("Path");
    QDomText pathText = doc.createTextNode(m_imagePath);
    path.appendChild(pathText);
    image.appendChild(path);
    QDomElement display = doc.createElement("Display");
    display.setAttribute("width", m_displaySize.width);
    display.setAttribute("widthUnit", unitNames[m_displaySize.widthUnit]);
    display.setAttribute("height", m_displaySize.height);
    display.setAttribute("heightUnit", unitNames[m_displaySize.heightUnit]);
    image.appendChild(display);
    QDomElement print = doc.createElement("Print");
    print.setAttribute("useDisplaySize", m_useDisplaySizeForPrinting);
    print.setAttribute("width", m_printSize.width);
    print.setAttribute("widthUnit", unitNames[m_printSize.widthUnit]);
    print.setAttribute("height", m_printSize.height);
    print.setAttribute("heightUnit", unitNames[m_printSize.heightUnit]);
    image.appendChild(print);

    // For the conversion to latex
    QDomElement latexSize = doc.createElement("LatexSizeString");
    QString sizeString;
    if (m_useDisplaySizeForPrinting)
        sizeString = latexSizeString(m_displaySize);
    else
        sizeString = latexSizeString(m_printSize);
    QDomText latexSizeString = doc.createTextNode(sizeString);
    latexSize.appendChild(latexSizeString);
    image.appendChild(latexSize);

    return image;
}

QString ImageEntry::toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq)
{
    Q_UNUSED(commandSep);

    return commentStartingSeq + "image: " + m_imagePath  + commentEndingSeq;
}

QString ImageEntry::latexSizeString(const ImageSize& imgSize)
{
    // We use the transformation 1 px = 1/72 in ( = 1 pt in Latex)

    QString sizeString="";
    if (imgSize.widthUnit == ImageSize::Auto &&
        imgSize.heightUnit == ImageSize::Auto)
        return QString("");

    if (imgSize.widthUnit == ImageSize::Percent) {
        if (imgSize.heightUnit == ImageSize::Auto ||
            (imgSize.heightUnit == ImageSize::Percent &&
             imgSize.width == imgSize.height))
            return "[scale=" + QString::number(imgSize.width / 100) + "]";
        // else? We could set the size based on the actual image size
    } else if (imgSize.widthUnit == ImageSize::Auto &&
               imgSize.heightUnit == ImageSize::Percent) {
        return "[scale=" + QString::number(imgSize.height / 100) + "]";
    }

    if (imgSize.heightUnit == ImageSize::Pixel)
        sizeString = "height=" + QString::number(imgSize.height) + "pt";
    if (imgSize.widthUnit == ImageSize::Pixel) {
        if (!sizeString.isEmpty())
            sizeString += ",";
        sizeString += "width=" + QString::number(imgSize.width) + "pt";
    }
    return "[" + sizeString + "]";
}

void ImageEntry::interruptEvaluation()
{
}

bool ImageEntry::evaluate(EvaluationOption evalOp)
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
    if (imgSize.heightUnit == ImageSize::Percent)
        h = srcSize.height() * imgSize.height / 100;
    else if (imgSize.heightUnit == ImageSize::Pixel)
        h = imgSize.height;
    if (imgSize.widthUnit == ImageSize::Percent)
        w = srcSize.width() * imgSize.width / 100;
    else if (imgSize.widthUnit == ImageSize::Pixel)
        w = imgSize.width;

    if (imgSize.widthUnit == ImageSize::Auto) {
        if (imgSize.heightUnit == ImageSize::Auto)
            return QSizeF(srcSize.width(), srcSize.height());
        else if (h == 0)
            w = 0;
        else
            w = h / srcSize.height() * srcSize.width();
    } else if (imgSize.heightUnit == ImageSize::Auto) {
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

void ImageEntry::addActionsToBar(ActionBar* actionBar)
{
    actionBar->addButton(KIcon("configure"), i18n("Configure Image"),
                         this, SLOT(startConfigDialog()));
}


void ImageEntry::layOutForWidth(double w, bool force)
{
    if (size().width() == w && !force)
        return;

    if (m_imageItem && m_imageItem->isVisible())
        m_imageItem->setGeometry(0, 0, w, true);
    else
        m_textItem->setGeometry(0, 0, w, true);

    setSize(QSizeF(w, height() + VerticalMargin));
}

bool ImageEntry::wantToEvaluate()
{
    return false;
}

bool ImageEntry::wantFocus()
{
    return false;
}

#include "imageentry.moc"
