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
    Copyright (C) 2019 Sirgienko Nikita <warquark@gmail.com>
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
