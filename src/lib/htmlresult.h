/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2019 Sirgienko Nikita <warquark@gmail.com>
*/

#ifndef _HTMLRESULT_H
#define _HTMLRESULT_H

#include <QJsonValue>

#include "result.h"
#include "cantor_export.h"

namespace Cantor
{

class HtmlResultPrivate;
/**
 * Class for html results
 * Instead of TextResult supports show/hide source html code like LatexResult
 * Also the result allows see plain alternative of the html, if available
 */
class CANTOR_EXPORT HtmlResult : public Result
{
  public:
    enum { Type=8 };
    enum Format { Html, HtmlSource, PlainAlternative};
    HtmlResult(const QString& html, const QString& plain = QString(), const std::map<QString, QJsonValue>& alternatives = std::map<QString, QJsonValue>());
    ~HtmlResult() override;

    QString toHtml() override;
    QVariant data() override;
    QString plain();

    void setFormat(Format format);
    Format format();

    int type() override;
    QString mimeType() override;

    QDomElement toXml(QDomDocument& doc) override;
    QJsonValue toJupyterJson() override;

    void save(const QString& filename) override;

  private:
    HtmlResultPrivate* d;
};

}
#endif /* _HTMLRESULT_H */
