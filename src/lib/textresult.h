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

#ifndef _TEXTRESULT_H
#define _TEXTRESULT_H

#include "result.h"

#include "mathematik_export.h"

namespace MathematiK
{

class TextResultPrivate;
class MATHEMATIK_EXPORT TextResult : public Result
{
  public:
    enum { Type=1 };
    enum Format { PlainTextFormat, LatexFormat};
    TextResult(const QString& text);
    ~TextResult();

    QString toHtml();
    QVariant data();

    int type();

    Format format();
    void setFormat(Format f);

    QDomElement toXml(QDomDocument& doc);

  private:
    TextResultPrivate* d;
};

}
#endif /* _TEXTRESULT_H */
