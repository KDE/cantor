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
    Copyright (C) 2019 Nikita Sirgienko <warquark@gmail.com>
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
