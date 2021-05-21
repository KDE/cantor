/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2019 Nikita Sirgienko <warquark@gmail.com>
*/

#ifndef _MIMERESULT_H
#define _MIMERESULT_H

#include <QJsonObject>

#include "result.h"
#include "cantor_export.h"

namespace Cantor
{

class MimeResultPrivate;

/**
 * Class for Jupyter results, which can't be handeled by Cantor
 * So data of the results and their mime types stored in this result
 * for preventing loosing
 * This must be used only with Jupyter notebook results with unsupported mime type
 */
class CANTOR_EXPORT MimeResult : public Result
{
  public:
    enum { Type = 4 };
    MimeResult(const QJsonObject& mimeBundle);
    ~MimeResult() override;

    QString toHtml() override;

    QVariant data() override;
    QString plain();

    int type() override;
    QString mimeType() override;

    QDomElement toXml(QDomDocument& doc) override;

    QJsonValue toJupyterJson() override;
    void save(const QString& filename) override;
  private:
    MimeResultPrivate* d;
};

}

#endif /* _MIMERESULT_H */
