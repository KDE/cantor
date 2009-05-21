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

#ifndef _EPSRESULT_H
#define _EPSRESULT_H

#include "result.h"
#include "mathematik_export.h"
#include "kurl.h"

namespace MathematiK
{
class EpsResultPrivate;

class MATHEMATIK_EXPORT EpsResult : public Result
{
  public:
    enum {Type=5};
    EpsResult( const KUrl& url);
    ~EpsResult();

    QString toHtml();
    QVariant data();

    int type();
    QDomElement toXml(QDomDocument& doc);
    void saveAdditionalData(KZip* archive);

  private:
    EpsResultPrivate* d;
};

}

#endif /* _EPSRESULT_H */
