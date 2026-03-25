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

QDomElement PdfResult::toXml(QDomDocument& doc)
{
    QDomElement e = doc.createElement(QStringLiteral("Result"));
    e.setAttribute(QStringLiteral("type"), QStringLiteral("pdf"));
    e.setAttribute(QStringLiteral("filename"), d->url.fileName());
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

    root.insert(QLatin1String("metadata"), jupyterMetadata());
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
