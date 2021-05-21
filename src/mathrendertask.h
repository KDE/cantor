/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2019 Sirgienko Nikita <warquark@gmail.com>
*/
#ifndef MATHRENDERTASK_H
#define MATHRENDERTASK_H

#include <QObject>
#include <QString>
#include <QTextImageFormat>
#include <QUrl>
#include <QImage>
#include <QRunnable>
#include <QSharedPointer>

#include "lib/latexrenderer.h"

class QMutex;

struct MathRenderResult
{
    int jobId;
    bool successful;
    QString errorMessage;
    QTextImageFormat renderedMath;
    QUrl uniqueUrl;
    QImage image;
};
Q_DECLARE_METATYPE(MathRenderResult)
Q_DECLARE_METATYPE(QSharedPointer<MathRenderResult>)

class MathRenderTask : public QObject, public QRunnable
{
  Q_OBJECT
  public:
    MathRenderTask(
        int jobId,
        const QString& code,
        Cantor::LatexRenderer::EquationType type,
        double scale,
        bool highResolution
    );

    void setHandler(const QObject *receiver, const char *resultHandler);

    void run() override;

    static std::pair<QTextImageFormat, QImage> renderPdfToFormat(
        const QString& filename,
        const QString& code,
        const QString uuid,
        Cantor::LatexRenderer::EquationType type,
        double scale,
        bool highResulution,
        bool* success = nullptr,
        QString* errorReason = nullptr
    );

    static QString genUuid();

  Q_SIGNALS:
    void finish(QSharedPointer<MathRenderResult> result);

  private:
    void finalize(QSharedPointer<MathRenderResult> result);

  private:
    int m_jobId;
    QString m_code;
    Cantor::LatexRenderer::EquationType m_type;
    double m_scale;
    bool m_highResolution;
    QColor m_backgroundColor;
    QColor m_foregroundColor;

};

#endif /* MATHRENDERTASK_H */
