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

#ifndef _HELPRESULT_H
#define _HELPRESULT_H

#include "textresult.h"

namespace Cantor
{

/** this is basically a TextResult, just with a different Type
    so that the application can show it in another way than the
    normal results
**/
class HelpResultPrivate;
class CANTOR_EXPORT HelpResult: public Result
{
  public:
    enum {Type=3};
    explicit HelpResult( const QString& text, bool isHtml=false);
    ~HelpResult() override = default;

    QVariant data() override;
    QString toHtml() override;

    int type() override;
    QString mimeType() override;

    QDomElement toXml(QDomDocument& doc) override;
    void save(const QString& filename) override;

  private:
    HelpResultPrivate* d;
};

}

#endif /* _HELPRESULT_H */
