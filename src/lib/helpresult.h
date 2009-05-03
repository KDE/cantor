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

namespace MathematiK
{

/** this is basically a TextResult, just with a different Type
    so that the application can show it in another way than the
    normal results
**/
class MATHEMATIK_EXPORT HelpResult : public TextResult
{
  public:
    enum {Type=3};
    HelpResult( const QString& text);
    ~HelpResult();

    int type();

    QDomElement toXml(QDomDocument& doc);
};

class ContextHelpResultPrivate;
class MATHEMATIK_EXPORT ContextHelpResult : public Result
{
  public:
    enum {Type=4};
    ContextHelpResult( const QStringList& text );
    ~ContextHelpResult();

    QString toHtml();
    QVariant data();

    int type();

    QDomElement toXml(QDomDocument& doc);
  private:
    ContextHelpResultPrivate* d;
};

};

#endif /* _HELPRESULT_H */
