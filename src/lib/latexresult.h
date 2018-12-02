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

#ifndef _LATEXRESULT_H
#define _LATEXRESULT_H

#include "epsresult.h"
#include "cantor_export.h"

namespace Cantor{
class LatexResultPrivate;

/**Class used for LaTeX results, it is basically an Eps result,
   but it exports a different type, and additionally stores the
   LaTeX code, used to generate the Eps, so it can be retrieved
   later
**/
class CANTOR_EXPORT LatexResult : public EpsResult
{
  public:
    enum {Type=7};
    LatexResult( const QString& code, const QUrl& url, const QString& plain = QString());
    ~LatexResult() override;
    
    int type() override;
    QString mimeType() override;

    bool isCodeShown();
    void showCode();
    void showRendered();

    QString code();
    QString plain();

    QString toHtml() override;
    QString toLatex() override;
    QVariant data() override;

    QDomElement toXml(QDomDocument& doc) override;
    
    void save(const QString& filename) override;

  private:
    LatexResultPrivate* d;
};

}

#endif /* _LATEXRESULT_H */
