/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
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
    LatexResult( const QString& code, const QUrl& url, const QString& plain = QString(), const QImage& image = QImage());
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
    QJsonValue toJupyterJson() override;

    void save(const QString& filename) override;

  private:
    LatexResultPrivate* d;
};

}

#endif /* _LATEXRESULT_H */
