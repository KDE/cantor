#ifndef _PDFRESULT_H
#define _PDFRESULT_H

#include "result.h"
#include <QUrl>
#include <QByteArray>
#include <QImage>

namespace Cantor
{
    class PdfResultPrivate;

    class CANTOR_EXPORT PdfResult : public Result
    {
    public:
        enum { Type = 8 };
        explicit PdfResult(const QUrl& url, const QByteArray& pdfData);
        ~PdfResult() override;

        QString toHtml() override;
        QString toLatex() override;
        QVariant data() override;
        QUrl url() override;

        int type() override;
        QString mimeType() override;

        QDomElement toXml(QDomDocument& doc) override;
        QJsonValue toJupyterJson() override;
        void saveAdditionalData(KZip* archive) override;
        void save(const QString& filename) override;

        QImage renderToImage(double scale, bool useHighRes = true);

        QByteArray pdfData() const;

    private:
        PdfResultPrivate* d;
    };

}

#endif /* _PDFRESULT_H */
