/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2022 by Alexander Semke (alexander.semke@web.de)
*/

#ifndef _IMAGERESULT_H
#define _IMAGERESULT_H

#include "result.h"
#include <QUrl>
#include <QSize>

class QImage;

namespace Cantor
{
class ImageResultPrivate;

class CANTOR_EXPORT ImageResult : public Result
{
  public:
    enum{Type=2};
    explicit ImageResult( const QUrl& url, const QString& alt=QString());
    explicit ImageResult( const QImage& image, const QString& alt=QString());
    ~ImageResult() override;

    QString toHtml() override;
    QString toLatex() override;
    QVariant data() override;
    QUrl url() override;

    int type() override;
    QString mimeType() override;
    QString extension();

    QSize displaySize();
    void setDisplaySize(QSize size);

    QString originalFormat();
    void setOriginalFormat(const QString& format);
    void setSvgContent(const QString& svgContent);

    QDomElement toXml(QDomDocument& doc) override;
    QJsonValue toJupyterJson() override;
    void saveAdditionalData(KZip* archive) override;

    void save(const QString& filename) override;

  private:
    ImageResultPrivate* d;
};

}

#endif /* _IMAGERESULT_H */
