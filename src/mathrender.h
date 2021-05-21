/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2019 Sirgienko Nikita <warquark@gmail.com>
*/
#ifndef MATHRENDER_H
#define MATHRENDER_H

#include <QObject>
#include <QTextImageFormat>
#include <QMutex>

#include "lib/latexrenderer.h"

/**
 * Special class for rendering embedded math in MarkdownEntry and TextEntry
 * Instead of LatexRenderer+EpsRenderer provide all needed functianality in one class
 * Even if we add some speed optimization in future, API of the class probably won't change
 */
class MathRenderer : public QObject {
  Q_OBJECT
  public:

    MathRenderer();
    ~MathRenderer();

    static bool mathRenderAvailable();

    // Resulution contol
    void setScale(qreal scale);
    qreal scale();
    void useHighResolution(bool b);

    /**
     * This function will run render task in Qt thread pool and
     * call resultHandler SLOT with MathRenderResult* argument on finish
     * receiver will be managed about pointer, task only create it
     */
    void renderExpression(
        int jobId,
        const QString& mathExpression,
        Cantor::LatexRenderer::EquationType type,
        const QObject *receiver,
        const char *resultHandler);


    /**
     * Rerender renderer math expression in document
     * Unlike MathRender::renderExpression this method isn't async, because
     * rerender already rendered math is not long operation
     */
    void rerender(QTextDocument* document, const QTextImageFormat& math);

    /**
     * Render math expression from existing .pdf
     * Like MathRenderer::rerender is blocking
     */
    std::pair<QTextImageFormat, QImage> renderExpressionFromPdf(
        const QString& filename, const QString& uuid, const QString& code, Cantor::LatexRenderer::EquationType type, bool* success
    );

  private:
    double m_scale;
    bool m_useHighRes;
};

#endif /* MATHRENDER_H */
