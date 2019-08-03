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

#include "mathrendertask.h"

#include <QTemporaryFile>
#include <QStandardPaths>
#include <QUuid>
#include <QDir>
#include <KColorScheme>
#include <KProcess>
#include <QScopedPointer>
#include <QMutex>
#include <QApplication>
#include <QDebug>

#include <poppler-qt5.h>

#include "epsrenderer.h"

static const QLatin1String mathTex("\\documentclass{standalone}"\
                         "\\usepackage{amsfonts,amssymb}"\
                         "\\usepackage{amsmath}"\
                         "\\usepackage[utf8]{inputenc}"\
                         "\\usepackage{color}"\
                         /*
                         "\\setlength\\textwidth{5in}"\
                         "\\setlength{\\parindent}{0pt}"\
                         "\\pagestyle{empty}"\
                         */
                         "\\begin{document}"\
                         "\\pagecolor[rgb]{%1,%2,%3}"\
                         "\\color[rgb]{%4,%5,%6}"\
                         "\\fontsize{%7}{%7}\\selectfont"\
                         "%8"\
                         "\\end{document}");

static const QLatin1String eqnHeader("$\\displaystyle %1$");
static const QLatin1String inlineEqnHeader("$%1$");

MathRenderTask::MathRenderTask(
        int jobId,
        const QString& code,
        Cantor::LatexRenderer::EquationType type,
        double scale,
        bool highResolution,
        QMutex* mutex
    ): m_jobId(jobId), m_code(code), m_type(type), m_scale(scale), m_highResolution(highResolution), m_mutex(mutex)
    {}

void MathRenderTask::setHandler(const QObject* receiver, const char* resultHandler)
{
    connect(this, SIGNAL(finish(QSharedPointer<MathRenderResult>)), receiver, resultHandler);
}

void MathRenderTask::run()
{
    QSharedPointer<MathRenderResult> result(new MathRenderResult());

    const QString& tempDir=QStandardPaths::writableLocation(QStandardPaths::TempLocation);

    QTemporaryFile texFile(tempDir + QDir::separator() + QLatin1String("cantor_tex-XXXXXX.tex"));
    texFile.open();

    // Verify that standalone.cls available for rendering and could be founded
    if (!tempDir.contains(QLatin1String("standalone.cls")))
    {
        QString file = QStandardPaths::locate(QStandardPaths::AppDataLocation, QLatin1String("latex/standalone.cls"));

        if (file.isEmpty())
            file = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("cantor/latex/standalone.cls"));

        if (file.isEmpty())
        {
            result->successfull = false;
            result->errorMessage = QString::fromLatin1("needed for math render standalone.cls file not found in Cantor data directory");
            finalize(result);
            return;
        }
        else
            QFile::copy(file, tempDir + QDir::separator() + QLatin1String("standalone.cls"));
    }
    QString expressionTex=mathTex;

    KColorScheme scheme(QPalette::Active);
    const QColor &backgroundColor=scheme.background().color();
    const QColor &foregroundColor=scheme.foreground().color();

    expressionTex=expressionTex
                            .arg(backgroundColor.redF()).arg(backgroundColor.greenF()).arg(backgroundColor.blueF())
                            .arg(foregroundColor.redF()).arg(foregroundColor.greenF()).arg(foregroundColor.blueF());

    int fontPointSize = QApplication::font().pointSize();
    expressionTex=expressionTex.arg(fontPointSize);

    switch(m_type)
    {
        case Cantor::LatexRenderer::FullEquation: expressionTex=expressionTex.arg(eqnHeader); break;
        case Cantor::LatexRenderer::InlineEquation: expressionTex=expressionTex.arg(inlineEqnHeader); break;
    }
    expressionTex=expressionTex.arg(m_code);

    texFile.write(expressionTex.toUtf8());
    texFile.flush();

    QProcess p;
    p.setWorkingDirectory(tempDir);

    // Create unique uuid for this job
    // It will be used as pdf filename, for preventing names collisions
    // And as internal url path too
    const QString& uuid = genUuid();

    const QString& pdflatex = QStandardPaths::findExecutable(QLatin1String("pdflatex"));
    p.setProgram(pdflatex);
    p.setArguments({QStringLiteral("-jobname=cantor_") + uuid, QStringLiteral("-halt-on-error"), texFile.fileName()});

    p.start();
    p.waitForFinished();

    if (p.exitCode() != 0)
    {
        // pdflatex render failed and we haven't pdf file
        result->successfull = false;

        QString renderErrorText = QString::fromUtf8(p.readAllStandardOutput());
        renderErrorText.remove(0, renderErrorText.indexOf(QLatin1Char('!')));
        renderErrorText.remove(renderErrorText.indexOf(QLatin1String("!  ==> Fatal error occurred")), renderErrorText.size());
        renderErrorText = renderErrorText.trimmed();
        result->errorMessage = renderErrorText;

        finalize(result);
        texFile.setAutoRemove(false); //Usefull for debug
        return;
    }

    //Clean up .aux and .log files
    QString pathWithoutExtention = tempDir + QDir::separator() + QLatin1String("cantor_")+uuid;
    QFile::remove(pathWithoutExtention + QLatin1String(".log"));
    QFile::remove(pathWithoutExtention + QLatin1String(".aux"));

    const QString& pdfFileName = pathWithoutExtention + QLatin1String(".pdf");

    bool success; QString errorMessage; QSizeF size;
    result->image = renderPdf(pdfFileName, m_scale, m_highResolution, &success, &size, &errorMessage, m_mutex);
    result->successfull = success;
    result->errorMessage = errorMessage;

    if (success == false)
    {
        finalize(result);
        return;
    }

    const auto& data = renderPdfToFormat(pdfFileName, m_code, uuid, m_type, m_scale, m_highResolution, &success, &errorMessage, m_mutex);
    result->successfull = success;
    result->errorMessage = errorMessage;
    if (success == false)
    {
        finalize(result);
        return;
    }

    result->renderedMath = data.first;
    result->image = data.second;
    result->jobId = m_jobId;

    QUrl internal;
    internal.setScheme(QLatin1String("internal"));
    internal.setPath(uuid);
    result->uniqueUrl = internal;

    finalize(result);
}

void MathRenderTask::finalize(QSharedPointer<MathRenderResult> result)
{
    emit finish(result);
    deleteLater();
}

QImage MathRenderTask::renderPdf(const QString& filename, double scale, bool highResolution, bool* success, QSizeF* size, QString* errorReason, QMutex* mutex)
{
    if (mutex)
        mutex->lock();
    Poppler::Document* document = Poppler::Document::load(filename);
    if (mutex)
        mutex->unlock();
    if (document == nullptr)
    {
        if (success)
            *success = false;
        if (errorReason)
            *errorReason = QString::fromLatin1("Poppler library have failed to open file % as pdf").arg(filename);
        return QImage();
    }

    Poppler::Page* pdfPage = document->page(0);
    if (pdfPage == nullptr) {
        if (success)
            *success = false;
        if (errorReason)
            *errorReason = QString::fromLatin1("Poppler library failed to access first page of %1 document").arg(filename);

        delete document;
        return QImage();
    }

    QSize pageSize = pdfPage->pageSize();

    double realScale = 1.7 * 1.8;
    qreal w = 1.7 * pageSize.width();
    qreal h = 1.7 * pageSize.height();
    if(highResolution)
        realScale *= 5;
    else
        realScale *= scale;


    QImage image = pdfPage->renderToImage(72.0*realScale, 72.0*realScale);

    delete pdfPage;
    if (mutex)
        mutex->lock();
    delete document;
    if (mutex)
        mutex->unlock();

    if (image.isNull())
    {
        if (success)
            *success = false;
        if (errorReason)
            *errorReason = QString::fromLatin1("Poppler library failed to render pdf %1 to image").arg(filename);

        return image;
    }

    // Resize with smooth transformation for more beautiful result
    image = image.convertToFormat(QImage::Format_ARGB32).scaled(image.size()/1.8, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    if (success)
        *success = true;

    if (size)
        *size = QSizeF(w, h);
    return image;
}

std::pair<QTextImageFormat, QImage> MathRenderTask::renderPdfToFormat(const QString& filename, const QString& code, const QString uuid, Cantor::LatexRenderer::EquationType type, double scale, bool highResulution, bool* success, QString* errorReason, QMutex* mutex)
{
    QSizeF size;
    const QImage& image = renderPdf(filename, scale, highResulution, success, &size, errorReason, mutex);

    if (success && *success == false)
        return std::make_pair(QTextImageFormat(), QImage());

    QTextImageFormat format;

    QUrl internal;
    internal.setScheme(QLatin1String("internal"));
    internal.setPath(uuid);

    format.setName(internal.url());
    format.setWidth(size.width());
    format.setHeight(size.height());
    format.setProperty(Cantor::EpsRenderer::CantorFormula, type);
    format.setProperty(Cantor::EpsRenderer::ImagePath, filename);
    format.setProperty(Cantor::EpsRenderer::Code, code);
    format.setVerticalAlignment(QTextCharFormat::AlignBaseline);

    switch(type)
    {
        case Cantor::LatexRenderer::FullEquation:
            format.setProperty(Cantor::EpsRenderer::Delimiter, QLatin1String("$$"));
            break;

        case Cantor::LatexRenderer::InlineEquation:
            format.setProperty(Cantor::EpsRenderer::Delimiter, QLatin1String("$"));
            break;
    }

    return std::make_pair(std::move(format), std::move(image));
}

QString MathRenderTask::genUuid()
{
    QString uuid = QUuid::createUuid().toString();
    uuid.remove(0, 1);
    uuid.chop(1);
    uuid.replace(QLatin1Char('-'), QLatin1Char('_'));
    return uuid;
}
