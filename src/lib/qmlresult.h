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
    Copyright (C) 2012 Alexander Rieder <alexanderrieder@gmail.com>
 */

#ifndef _QMLRESULT_H
#define _QMLRESULT_H

#include "result.h"

namespace Cantor{

struct ContextProperty{
    QString name;
    QVariant value;
};

typedef QList<ContextProperty> ContextPropertyList;

class QmlResultPrivate;

class QmlResult : public Result
{
  public:
    enum { Type = 8 };
    QmlResult( const QString& qml, const ContextPropertyList& properties);
    ~QmlResult();

    QString toHtml();
    QVariant data();

    QString plain();

    int type();
    QString mimeType();

    QDomElement toXml(QDomDocument& doc);

    void save(const QString& filename);

    void saveAdditionalData(KZip* archive);

  private:
    QImage renderToImage();
  private:
    QmlResultPrivate* d;

};

}

#endif /* _QMLRESULT_H */
