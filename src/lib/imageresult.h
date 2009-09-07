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
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
 */

#ifndef _IMAGERESULT_H
#define _IMAGERESULT_H

#include "result.h"

#include <kurl.h>

namespace MathematiK
{
class ImageResultPrivate;

class MATHEMATIK_EXPORT ImageResult : public Result
{
  public:
    enum{Type=2};
    explicit ImageResult( const KUrl& url, const QString& alt=QString());
    ~ImageResult();

    QString toHtml();
    QVariant data();

    int type();
    QString mimeType();

    QDomElement toXml(QDomDocument& doc);
    void saveAdditionalData(KZip* archive);

    void save(const QString& filename);

  private:
    ImageResultPrivate* d;
};

}

#endif /* _IMAGERESULT_H */
