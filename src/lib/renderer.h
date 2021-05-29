/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#ifndef RENDERER_H
#define RENDERER_H

#include <QTextDocument>
#include <QTextImageFormat>
#include <QPixmap>
#include <QSizeF>
#include <QUrl>
#include "latexrenderer.h"

namespace Cantor
{
class RendererPrivate;

class CANTOR_EXPORT Renderer
{
  public:
    Renderer();
    ~Renderer();

    enum FormulaProperties {CantorFormula = 1, ImagePath = 2, Code = 3,
                            Delimiter = 4};
    enum FormulaType {LatexFormula = Cantor::LatexRenderer::LatexMethod,
                      MmlFormula = Cantor::LatexRenderer::MmlMethod};
    enum Method {PDF, EPS};

    QTextImageFormat render(QTextDocument *document, const Cantor::LatexRenderer* latex);
    QTextImageFormat render(QTextDocument *document, Method method, const QUrl& url, const QString& uuid);

    void setScale(qreal scale);
    qreal scale();

    void useHighResolution(bool b);

    QSizeF renderToResource(QTextDocument *document, Method method, const QUrl& url, const QUrl& internal);

    QImage renderToImage(const QUrl& url, Method method, QSizeF* size = nullptr);
    static QImage epsRenderToImage(const QUrl& url, double scale, bool useHighRes, QSizeF* size = nullptr, QString* errorReason = nullptr);
    static QImage pdfRenderToImage(const QUrl& url, double scale, bool useHighRes, QSizeF* size = nullptr, QString* errorReason = nullptr);

  private:
    RendererPrivate* d;
};

}

#endif //RENDERER_H
