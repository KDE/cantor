/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 martin Kuettler <martin.kuettler@gmail.com>
*/

#include "imageentry.h"
#include "actionbar.h"
#include "worksheetimageitem.h"
#include "worksheetview.h"
#include "lib/jupyterutils.h"

#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QFileSystemWatcher>
#include <QJsonValue>
#include <QJsonObject>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTextOption>
#include <QUuid>

#include <KLocalizedString>
#include <KZip>

ImageEntry::ImageEntry(Worksheet* worksheet) : WorksheetEntry(worksheet)
{
    m_imageItem = nullptr;
    m_textItem = new WorksheetTextItem(this);
    QTextOption textOption = m_textItem->document()->defaultTextOption();
    textOption.setAlignment(Qt::AlignHCenter);
    m_textItem->document()->setDefaultTextOption(textOption);
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
    connect(m_imageWatcher, &QFileSystemWatcher::fileChanged, this, &ImageEntry::updateEntry);

    setFlag(QGraphicsItem::ItemIsFocusable);
    updateEntry();
}

ImageEntry::~ImageEntry()
{
    clearInternalImage();
}

void ImageEntry::populateMenu(QMenu* menu, QPointF pos)
{
    WorksheetEntry::populateMenu(menu, pos);
    auto* firstAction = menu->actions().at(0);

    auto* action = new QAction(QIcon::fromTheme(QLatin1String("configure")), i18n("Configure Image"));
    menu->insertAction(firstAction, action);
    connect(action, &QAction::triggered, this, &ImageEntry::startConfigDialog);
    menu->insertSeparator(firstAction);
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
    QDomElement fileName = content.firstChildElement(QLatin1String("FileName"));
    if (!fileName.isNull()) {
        const KArchiveEntry* imageEntry = file.directory()->entry(fileName.text());
        if (imageEntry && imageEntry->isFile())
        {
            const KArchiveFile* imageFile = static_cast<const KArchiveFile*>(imageEntry);
            QTemporaryDir temporaryDirectory(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QDir::separator() + QLatin1String("cantor-imageentry-XXXXXX"));
            if (temporaryDirectory.isValid() && imageFile->copyTo(temporaryDirectory.path()))
            {
                clearInternalImage();
                m_archiveTempDirPath = temporaryDirectory.path();
                temporaryDirectory.setAutoRemove(false);
                m_internalImagePath = temporaryDirectory.filePath(imageFile->name());
                if (!m_userImagePath.isEmpty() && m_imageWatcher->files().contains(m_userImagePath))
                    m_imageWatcher->removePath(m_userImagePath);
                m_userImagePath.clear();
            }
        }
    } else {
        // to support the legacy way
        QDomElement pathElement = content.firstChildElement(QLatin1String("Path"));
        QString path = pathElement.text();
        if (!path.isEmpty())
        {
            clearInternalImage();

            if (!m_userImagePath.isEmpty() && m_imageWatcher->files().contains(m_userImagePath))
                m_imageWatcher->removePath(m_userImagePath);
            m_userImagePath = path;
        }
    }

    loadSizeProperties(content);
    updateEntry();
}

void ImageEntry::setContent(const QDomElement& content)
{
    const QDomElement dataElement = content.firstChildElement(QLatin1String("Data"));
    const QByteArray data = QByteArray::fromBase64(dataElement.text().toLatin1());
    if (!data.isEmpty())
    {
        QString suffix = dataElement.attribute(QLatin1String("suffix"));
        if (suffix.isEmpty())
            suffix = QLatin1String("png");
        QTemporaryFile file(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QDir::separator() + QLatin1String("cantor-clipboard-XXXXXX.") + suffix);
        if (file.open() && file.write(data) == data.size())
        {
            file.close();
            file.setAutoRemove(false);
            QFileInfo fileInfo(file.fileName());
            clearInternalImage();
            m_internalImagePath = fileInfo.absoluteFilePath();
            if (!m_userImagePath.isEmpty() && m_imageWatcher->files().contains(m_userImagePath))
                m_imageWatcher->removePath(m_userImagePath);
            m_userImagePath.clear();
        }
    }

    loadSizeProperties(content);
    updateEntry();
}

void ImageEntry::loadSizeProperties(const QDomElement& content)
{
    static const QStringList unitNames = {QLatin1String("(auto)"), QLatin1String("px"), QLatin1String("%")};
    const QDomElement displayElement = content.firstChildElement(QLatin1String("Display"));
    const QDomElement printElement = content.firstChildElement(QLatin1String("Print"));
    m_displaySize.width = displayElement.attribute(QLatin1String("width")).toDouble();
    m_displaySize.height = displayElement.attribute(QLatin1String("height")).toDouble();
    m_displaySize.widthUnit = unitNames.indexOf(displayElement.attribute(QLatin1String("widthUnit")));
    m_displaySize.heightUnit = unitNames.indexOf(displayElement.attribute(QLatin1String("heightUnit")));
    m_useDisplaySizeForPrinting = printElement.attribute(QLatin1String("useDisplaySize")).toInt();
    m_printSize.width = printElement.attribute(QLatin1String("width")).toDouble();
    m_printSize.height = printElement.attribute(QLatin1String("height")).toDouble();
    m_printSize.widthUnit = unitNames.indexOf(printElement.attribute(QLatin1String("widthUnit")));
    m_printSize.heightUnit = unitNames.indexOf(printElement.attribute(QLatin1String("heightUnit")));
}

void ImageEntry::setContentFromJupyter(const QJsonObject& cell)
{
    // No need use ImageEntry because without file this entry type are useless
    Q_UNUSED(cell);
    return;
}

QJsonValue ImageEntry::toJupyterJson()
{
    QJsonValue value;

    if (!(m_userImagePath.isEmpty() && m_internalImagePath.isEmpty()) && m_imageItem)
    {
        const QImage& image = m_imageItem->pixmap().toImage();
        if (!image.isNull())
        {
            QJsonObject entry;
            entry.insert(QLatin1String("cell_type"), QLatin1String("markdown"));

            QJsonObject metadata;
            QJsonObject size;
            size.insert(QLatin1String("width"), image.size().width());
            size.insert(QLatin1String("height"), image.size().height());
            metadata.insert(Cantor::JupyterUtils::pngMime, size);
            entry.insert(Cantor::JupyterUtils::metadataKey, metadata);

            QString text(QLatin1String("<img src='attachment:image.png'>"));

            QJsonObject attachments;
            attachments.insert(QLatin1String("image.png"), Cantor::JupyterUtils::packMimeBundle(image, Cantor::JupyterUtils::pngMime));
            entry.insert(QLatin1String("attachments"), attachments);

            Cantor::JupyterUtils::setSource(entry, text);

            value = entry;
        }
    }

    return value;
}

QDomElement ImageEntry::toXml(QDomDocument& doc, KZip& archive)
{
    QDomElement image = doc.createElement(QLatin1String("Image"));

    const QString imagePath = m_userImagePath.isEmpty() ? m_internalImagePath : m_userImagePath;
    const QFileInfo imageInfo(imagePath);

    QString archiveName = QLatin1String("cantor_imageentry_") + QUuid::createUuid().toString(QUuid::WithoutBraces);

    const QString suffix = imageInfo.suffix();
    if (!suffix.isEmpty())
    {
        archiveName += QLatin1Char('.');
        archiveName += suffix;
    }

    const QString sourcePath = imageInfo.canonicalFilePath();
    if (!sourcePath.isEmpty() && archive.addLocalFile(sourcePath, archiveName))
    {
        QDomElement fileNameElement = doc.createElement(QLatin1String("FileName"));
        fileNameElement.appendChild(doc.createTextNode(archiveName));
        image.appendChild(fileNameElement);
    }

    appendSizeProperties(doc, image);

    QDomElement latexSizeElement = doc.createElement(QLatin1String("LatexSizeString"));
    const QString sizeString = m_useDisplaySizeForPrinting ? latexSizeString(m_displaySize) : latexSizeString(m_printSize);
    latexSizeElement.appendChild(doc.createTextNode(sizeString));
    image.appendChild(latexSizeElement);

    // Preserve the legacy Path consumed by latex.xsl; archive import uses FileName.
    QDomElement path = doc.createElement(QLatin1String("Path"));
    path.appendChild(doc.createTextNode(imagePath));
    image.appendChild(path);

    return image;
}

QDomElement ImageEntry::toXml(QDomDocument& doc)
{
    QDomElement image = doc.createElement(QLatin1String("Image"));
    QString sourcePath = m_userImagePath.isEmpty() ? m_internalImagePath : m_userImagePath;

    QFile file(sourcePath);
    if (file.open(QIODevice::ReadOnly))
    {
        QDomElement dataElement = doc.createElement(QLatin1String("Data"));
        dataElement.setAttribute(QLatin1String("name"), QFileInfo(sourcePath).fileName());
        dataElement.setAttribute(QLatin1String("suffix"), QFileInfo(sourcePath).suffix());
        dataElement.appendChild(doc.createTextNode(QString::fromLatin1(file.readAll().toBase64())));
        image.appendChild(dataElement);
    }

    appendSizeProperties(doc, image);
    return image;
}

void ImageEntry::appendSizeProperties(QDomDocument& doc, QDomElement& image) const
{
    static const QStringList unitNames = {QLatin1String("(auto)"), QLatin1String("px"), QLatin1String("%")};
    QDomElement displayElement = doc.createElement(QLatin1String("Display"));
    displayElement.setAttribute(QLatin1String("width"), m_displaySize.width);
    displayElement.setAttribute(QLatin1String("widthUnit"), unitNames.value(m_displaySize.widthUnit, unitNames[0]));
    displayElement.setAttribute(QLatin1String("height"), m_displaySize.height);
    displayElement.setAttribute(QLatin1String("heightUnit"), unitNames.value(m_displaySize.heightUnit, unitNames[0]));
    image.appendChild(displayElement);

    QDomElement printElement = doc.createElement(QLatin1String("Print"));
    printElement.setAttribute(QLatin1String("useDisplaySize"), m_useDisplaySizeForPrinting);
    printElement.setAttribute(QLatin1String("width"), m_printSize.width);
    printElement.setAttribute(QLatin1String("widthUnit"), unitNames.value(m_printSize.widthUnit, unitNames[0]));
    printElement.setAttribute(QLatin1String("height"), m_printSize.height);
    printElement.setAttribute(QLatin1String("heightUnit"), unitNames.value(m_printSize.heightUnit, unitNames[0]));
    image.appendChild(printElement);
}

QString ImageEntry::toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq)
{
    Q_UNUSED(commandSep);
    QString imagePath = m_userImagePath.isEmpty() ? m_internalImagePath : m_userImagePath;
    return commentStartingSeq + QLatin1String("image: ") + imagePath  + commentEndingSeq;
}

QString ImageEntry::latexSizeString(const ImageSize& imgSize)
{
    // We use the transformation 1 px = 1/72 in ( = 1 pt in Latex)

    QString sizeString=QLatin1String("");
    if (imgSize.widthUnit == ImageSize::Auto &&
        imgSize.heightUnit == ImageSize::Auto)
        return QLatin1String("");

    if (imgSize.widthUnit == ImageSize::Percent) {
        if (imgSize.heightUnit == ImageSize::Auto ||
            (imgSize.heightUnit == ImageSize::Percent &&
             imgSize.width == imgSize.height))
            return QLatin1String("[scale=") + QString::number(imgSize.width / 100) + QLatin1String("]");
        // else? We could set the size based on the actual image size
    } else if (imgSize.widthUnit == ImageSize::Auto &&
               imgSize.heightUnit == ImageSize::Percent) {
        return QLatin1String("[scale=") + QString::number(imgSize.height / 100) + QLatin1String("]");
    }

    if (imgSize.heightUnit == ImageSize::Pixel)
        sizeString = QLatin1String("height=") + QString::number(imgSize.height) + QLatin1String("pt");
    if (imgSize.widthUnit == ImageSize::Pixel) {
        if (!sizeString.isEmpty())
            sizeString += QLatin1String(",");
        sizeString += QLatin1String("width=") + QString::number(imgSize.width) + QLatin1String("pt");
    }
    return QLatin1String("[") + sizeString + QLatin1String("]");
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
    if (m_userImagePath.isEmpty() && m_internalImagePath.isEmpty()) {
        m_textItem->setPlainText(i18n("Double click here to configure image settings"));
        m_textItem->setVisible(true);
        if (m_imageItem)
            m_imageItem->setVisible(false);
    }

    else {
        if (!m_imageItem)
            m_imageItem = new WorksheetImageItem(this);

        QString imagePath = m_userImagePath.isEmpty() ? m_internalImagePath : m_userImagePath;
        // we no longer have an EPS renderer. The old EPS branch called setPdf(),
        // which is PDF-only after the removal of libspectre and cannot render EPS.
        m_imageItem->setImage(QImage(imagePath));

        if (!m_imageItem->imageIsValid()) {
            if (m_imageWatcher->files().contains(imagePath))
                m_imageWatcher->removePath(imagePath);
            const QString msg = i18n("Cannot load image %1", imagePath);
            m_textItem->setPlainText(msg);
            m_textItem->setVisible(true);
            m_imageItem->setVisible(false);
        } else {
            if (!m_imageWatcher->files().contains(imagePath))
                m_imageWatcher->addPath(imagePath);
            QSizeF size;
            if (worksheet()->isPrinting() && ! m_useDisplaySizeForPrinting)
                size = imageSize(m_printSize);
            else
                size = imageSize(m_displaySize);
            m_imageItem->setSize(size);
            m_textItem->setVisible(false);
            m_imageItem->setVisible(true);
        }
    }

    if (oldHeight != height())
        recalculateSize();
}

QSizeF ImageEntry::imageSize(const ImageSize& imgSize)
{
    const QSize& srcSize = m_imageItem->imageSize();
    qreal w = 0.0;
    qreal h = 0.0;
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
    QString imagePath = m_userImagePath.isEmpty() ? m_internalImagePath : m_userImagePath;
    ImageSettingsDialog* dialog = new ImageSettingsDialog(worksheet()->worksheetView());
    dialog->setData(imagePath, m_displaySize, m_printSize,
                    m_useDisplaySizeForPrinting);
    connect(dialog, &ImageSettingsDialog::dataChanged, this, &ImageEntry::setImageData);
    dialog->show();
}

void ImageEntry::setImageData(const QString& path,
                              const ImageSize& displaySize,
                              const ImageSize& printSize,
                              bool useDisplaySizeForPrinting)
{
    if (path.isEmpty())
    {
        if (!m_userImagePath.isEmpty() && m_imageWatcher->files().contains(m_userImagePath))
            m_imageWatcher->removePath(m_userImagePath);

        m_userImagePath.clear();
        clearInternalImage();
    }
    else {
        if (path != m_internalImagePath && path != m_userImagePath) {
            if (!m_userImagePath.isEmpty() && m_imageWatcher->files().contains(m_userImagePath))
                m_imageWatcher->removePath(m_userImagePath);
            m_userImagePath = path;
        }

        if (path != m_internalImagePath) {
            clearInternalImage();
        }
    }

    m_displaySize = displaySize;
    m_printSize = printSize;
    m_useDisplaySizeForPrinting = useDisplaySizeForPrinting;

    updateEntry();
}

void ImageEntry::addActionsToBar(ActionBar* actionBar)
{
    actionBar->addButton(QIcon::fromTheme(QLatin1String("configure")), i18n("Configure Image"),
                         this, SLOT(startConfigDialog()));
}

void ImageEntry::layOutForWidth(qreal entry_zone_x, qreal w, bool force)
{
    if (size().width() == w && m_textItem->pos().x() == entry_zone_x && !force)
        return;

    //TODO somethinkg wrong with geometry and control element: control element appears in wrong place
    const qreal margin = worksheet()->isPrinting() ? 0 : RightMargin;

    double width;
    if (m_imageItem && m_imageItem->isVisible()) {
        m_imageItem->setGeometry(entry_zone_x, 0, w - margin - entry_zone_x, true);
        width = m_imageItem->width();
    } else {
        m_textItem->setGeometry(entry_zone_x, 0, w - margin - entry_zone_x, false);
        width = m_textItem->width();
    }

    setSize(QSizeF(width + margin + entry_zone_x, height() + VerticalMargin));
}

bool ImageEntry::wantToEvaluate()
{
    return false;
}

bool ImageEntry::wantFocus()
{
    return false;
}

void ImageEntry::mouseDoubleClickEvent(QGraphicsSceneMouseEvent*)
{
    startConfigDialog();
}

void ImageEntry::clearInternalImage()
{
    if (!m_internalImagePath.isEmpty())
    {
        if (m_imageWatcher->files().contains(m_internalImagePath))
            m_imageWatcher->removePath(m_internalImagePath);
        m_internalImagePath.clear();
    }

    if (!m_archiveTempDirPath.isEmpty())
    {
        m_archiveTempDirPath.clear();
    }
}
