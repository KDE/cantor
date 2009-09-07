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

#ifndef _RESULT_H
#define _RESULT_H

#include <QVariant>
#include <QDomElement>
#include <kurl.h>
#include "mathematik_export.h"

class KZip;

namespace MathematiK
{

class ResultPrivate;

class MATHEMATIK_EXPORT Result
{
  public:
    Result( );
    virtual ~Result();

    virtual QString toHtml() = 0;
    virtual QVariant data() = 0;

    virtual int type() = 0;
    virtual QString mimeType() = 0;

    virtual QDomElement toXml(QDomDocument& doc) = 0;
    virtual void saveAdditionalData(KZip* archive);

    virtual void save(const QString& filename) = 0;
  private:
    ResultPrivate* d;
};

}

#endif /* _RESULT_H */
