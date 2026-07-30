#include "pdfresult.h"
#include "jupyterutils.h"

#include <poppler-qt6.h>
#include <QBuffer>
#include <QFile>
#include <QMutex>
#include <KZip>
#include <QJsonObject>
#include <QDebug>
#include <QScreen>

using namespace Cantor;

static QMutex popplerPdfMutex;

class Cantor::PdfResultPrivate
{
public:
    QUrl url;
    QByteArray pdfData;
    QSize displaySize;
};

PdfResult::PdfResult(const QUrl& url, const QByteArray& pdfData) : d(new PdfResultPrivate)
{
    d->url = url;
    d->pdfData = pdfData;
}

PdfResult::~PdfResult()
{
    delete d;
}

int PdfResult::type()
{
    return PdfResult::Type;
}

QString PdfResult::mimeType()
{
    return QStringLiteral("application/pdf");
}

QUrl PdfResult::url()
{
    return d->url;
}

QByteArray PdfResult::pdfData() const
{
    return d->pdfData;
}

QVariant PdfResult::data()
{
    return QVariant::fromValue(renderToImage(1.0));
}

QString PdfResult::toHtml()
{
    return QStringLiteral("<img src=\"%1\">").arg(d->url.url());
}

QString PdfResult::toLatex()
{
    return QStringLiteral("\\includegraphics{%1}").arg(d->url.toLocalFile());
}

QImage PdfResult::renderToImage(double scale, bool useHighRes)
{
    if (d->pdfData.isEmpty())
        return QImage();

    popplerPdfMutex.lock();
    auto document = Poppler::Document::loadFromData(d->pdfData);
    popplerPdfMutex.unlock();

    if (document == nullptr)
        return QImage();

    document->setRenderHint(Poppler::Document::Antialiasing, true);
    document->setRenderHint(Poppler::Document::TextAntialiasing, true);
    document->setRenderHint(Poppler::Document::TextHinting, true);

    auto pdfPage = document->page(0);
    if (pdfPage == nullptr)
        return QImage();

    double dpiX = QGuiApplication::primaryScreen()->physicalDotsPerInchX();
    double dpiScale = dpiX / 72.0;

    const double superSample = 2.0;
    double renderScale = dpiScale * superSample;

    if (useHighRes)
        renderScale *= 5;
    else
        renderScale *= scale;

    QImage image = pdfPage->renderToImage(72.0 * renderScale, 72.0 * renderScale);

    if (!image.isNull())
    {
        if (image.format() != QImage::Format_ARGB32_Premultiplied)
            image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);

        image.setDevicePixelRatio(superSample);
    }

    return image;
}

QImage PdfResult::renderToDisplaySize(const QSize& size)
{
    if (!size.isValid() || d->pdfData.isEmpty())
        return QImage();

    popplerPdfMutex.lock();
    auto document = Poppler::Document::loadFromData(d->pdfData);
    popplerPdfMutex.unlock();
    if (!document)
        return QImage();

    auto page = document->page(0);
    if (!page)
        return QImage();

    document->setRenderHint(Poppler::Document::Antialiasing, true);
    document->setRenderHint(Poppler::Document::TextAntialiasing, true);
    document->setRenderHint(Poppler::Document::TextHinting, true);

    constexpr qreal superSample = 2.0;
    const QSize renderSize(qMin(qRound(size.width() * superSample), 16384),
                           qMin(qRound(size.height() * superSample), 16384));
    const QSizeF pageSize = page->pageSizeF();
    if (!pageSize.isValid())
        return QImage();

    const qreal scale = qMax(renderSize.width() / pageSize.width(), renderSize.height() / pageSize.height());
    QImage image = page->renderToImage(72.0 * scale, 72.0 * scale);
    if (!image.isNull()) {
        if (image.format() != QImage::Format_ARGB32_Premultiplied)
            image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        image.setDevicePixelRatio(superSample);
    }
    return image;
}

QDomElement PdfResult::toXml(QDomDocument& doc)
{
    QDomElement e = doc.createElement(QStringLiteral("Result"));
    e.setAttribute(QStringLiteral("type"), QStringLiteral("pdf"));
    e.setAttribute(QStringLiteral("filename"), d->url.fileName());
    if (d->displaySize.isValid()) {
        e.setAttribute(QStringLiteral("display-width"), d->displaySize.width());
        e.setAttribute(QStringLiteral("display-height"), d->displaySize.height());
    }
    applyXmlResultMetadata(e);
    return e;
}

void PdfResult::saveAdditionalData(KZip* archive)
{
    archive->addLocalFile(d->url.toLocalFile(), d->url.fileName());
}

QJsonValue PdfResult::toJupyterJson()
{
    QJsonObject root;
    root.insert(QLatin1String("output_type"), QLatin1String("display_data"));

    QJsonObject data;
    data.insert(QLatin1String("application/pdf"), JupyterUtils::toJupyterMultiline(QString::fromLatin1(d->pdfData.toBase64())));
    root.insert(QLatin1String("data"), data);

    QJsonObject metadata = jupyterMetadata();
    if (d->displaySize.isValid()) {
        QJsonObject size;
        size.insert(QLatin1String("width"), d->displaySize.width());
        size.insert(QLatin1String("height"), d->displaySize.height());
        metadata.insert(QLatin1String("application/pdf"), size);
    }
    root.insert(QLatin1String("metadata"), metadata);
    return root;
}

void PdfResult::save(const QString& fileName)
{
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(d->pdfData);
        file.close();
    }
}

QSize PdfResult::displaySize() const
{
    return d->displaySize;
}

void PdfResult::setDisplaySize(const QSize& size)
{
    d->displaySize = size;
}
