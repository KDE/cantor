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
    Copyright (C) 2019 Sirgienko Nikita <warquark@gmail.com>
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
    bool successfull;
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
        bool highResolution,
        QMutex* mutex
    );

    void setHandler(const QObject *receiver, const char *resultHandler);

    void run() override;

    static QImage renderPdf(
        const QString& filename, double scale, bool highResulution, bool* success = nullptr, QSizeF* size = nullptr, QString* errorReason = nullptr, QMutex* mutex = nullptr
    );
    static std::pair<QTextImageFormat, QImage> renderPdfToFormat(
        const QString& filename,
        const QString& code,
        const QString uuid,
        Cantor::LatexRenderer::EquationType type,
        double scale,
        bool highResulution,
        bool* success = nullptr,
        QString* errorReason = nullptr,
        QMutex* mutex = nullptr
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
    QMutex* m_mutex;
};

#endif /* MATHRENDERTASK_H */
