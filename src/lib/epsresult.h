/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _EPSRESULT_H
#define _EPSRESULT_H

#include "result.h"
#include "cantor_export.h"
#include <QUrl>
#include <QImage>

namespace Cantor
{
class EpsResultPrivate;

class CANTOR_EXPORT EpsResult : public Result
{
  public:
    enum {Type=5};
    explicit EpsResult( const QUrl& url, const QImage& image = QImage());
    ~EpsResult() override;

    QString toHtml() override;
    QString toLatex() override;
    QVariant data() override;
    QUrl url() override;
    QImage image();

    int type() override;
    QString mimeType() override;

    QDomElement toXml(QDomDocument& doc) override;
    QJsonValue toJupyterJson() override;
    void saveAdditionalData(KZip* archive) override;

    void save(const QString& filename) override;

  private:
    EpsResultPrivate* d;
};

}

#endif /* _EPSRESULT_H */
