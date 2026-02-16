/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _LATEXRESULT_H
#define _LATEXRESULT_H

#include "imageresult.h"
#include "cantor_export.h"

#include <QImage>

namespace Cantor{
class LatexResultPrivate;

/**Class used for LaTeX results. It stores the image result of the rendered LaTeX code,
 a nd additionally stores the LaTeX code itself for later retrieval.
**/
class CANTOR_EXPORT LatexResult : public ImageResult
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
