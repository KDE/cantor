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
#include <QApplication>
#include <QDebug>

#include "lib/renderer.h"
#include "lib/latexrenderer.h"

static const QLatin1String mathTex("\\documentclass%9{minimal}"\
                         "\\usepackage{amsfonts,amssymb}"\
                         "\\usepackage{amsmath}"\
                         "\\usepackage[utf8]{inputenc}"\
                         "\\usepackage[active,displaymath,textmath,tightpage]{preview}"\
                         "\\usepackage{color}"\
                         "\\begin{document}"\
                         "\\begin{preview}"\
                         "$"\
                         "\\colorbox[rgb]{%1,%2,%3}{"\
                         "\\color[rgb]{%4,%5,%6}"\
                         "\\fontsize{%7}{%7}\\selectfont"\
                         "%8}"\
                         "$"\
                         "\\end{preview}"
                         "\\end{document}");

static const QLatin1String eqnHeader("$\\displaystyle %1$");
static const QLatin1String inlineEqnHeader("$%1$");

MathRenderTask::MathRenderTask(
        int jobId,
        const QString& code,
        Cantor::LatexRenderer::EquationType type,
        double scale,
        bool highResolution
    ): m_jobId(jobId), m_code(code), m_type(type), m_scale(scale), m_highResolution(highResolution)
{

    KColorScheme scheme(QPalette::Active);
    m_backgroundColor = scheme.background().color();
    m_foregroundColor = scheme.foreground().color();
}

void MathRenderTask::setHandler(const QObject* receiver, const char* resultHandler)
{
    connect(this, SIGNAL(finish(QSharedPointer<MathRenderResult>)), receiver, resultHandler);
}

void MathRenderTask::run()
{
    qDebug()<<"MathRenderTask::run " << m_jobId;
    QSharedPointer<MathRenderResult> result(new MathRenderResult());

    const QString& tempDir=QStandardPaths::writableLocation(QStandardPaths::TempLocation);

    QTemporaryFile texFile(tempDir + QDir::separator() + QLatin1String("cantor_tex-XXXXXX.tex"));
    texFile.open();

    // make sure we have preview.sty available
    if (!tempDir.contains(QLatin1String("preview.sty")))
    {
        QString file = QStandardPaths::locate(QStandardPaths::AppDataLocation, QLatin1String("latex/preview.sty"));

        if (file.isEmpty())
            file = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("cantor/latex/preview.sty"));

        if (file.isEmpty())
        {
            result->successful = false;
            result->errorMessage = QString::fromLatin1("LaTeX style file preview.sty not found.");
            finalize(result);
            return;
        }
        else
            QFile::copy(file, tempDir + QDir::separator() + QLatin1String("preview.sty"));
    }
    QString expressionTex=mathTex;

    expressionTex=expressionTex
                            .arg(m_backgroundColor.redF()).arg(m_backgroundColor.greenF()).arg(m_backgroundColor.blueF())
                            .arg(m_foregroundColor.redF()).arg(m_foregroundColor.greenF()).arg(m_foregroundColor.blueF());
    int fontPointSize = QApplication::font().pointSize();
    expressionTex=expressionTex.arg(fontPointSize);

    switch(m_type)
    {
        case Cantor::LatexRenderer::FullEquation:
            expressionTex=expressionTex.arg(eqnHeader, QString());
            break;
        case Cantor::LatexRenderer::InlineEquation:
            expressionTex=expressionTex.arg(inlineEqnHeader, QString());
            break;
        case Cantor::LatexRenderer::CustomEquation:
            expressionTex=expressionTex.arg(QLatin1String("%1"), QLatin1String("[preview]"));
            break;
    }

    QString latex = m_code;
    // Looks hacky, but no sure, how do it better without overhead (like new latex type in lib/latexrender)
    static const QString& equationBegin = QLatin1String("\\begin{equation}");
    static const QString& equationEnd = QLatin1String("\\end{equation}");
    if (latex.startsWith(equationBegin) && latex.endsWith(equationEnd))
    {
        latex.remove(0, equationBegin.size());
        latex.chop(equationEnd.size());
        latex = QLatin1String("\\begin{equation*}") + latex + QLatin1String("\\end{equation*}");
    }

    expressionTex=expressionTex.arg(latex);

    texFile.write(expressionTex.toUtf8());
    texFile.flush();

    QProcess p;
    p.setWorkingDirectory(tempDir);

    // Create unique uuid for this job
    // It will be used as pdf filename, for preventing names collisions
    // And as internal url path too
    const QString& uuid = Cantor::LatexRenderer::genUuid();

    const QString& pdflatex = QStandardPaths::findExecutable(QLatin1String("pdflatex"));
    p.setProgram(pdflatex);
    p.setArguments({QStringLiteral("-jobname=cantor_") + uuid, QStringLiteral("-halt-on-error"), texFile.fileName()});

    p.start();
    p.waitForFinished();

    if (p.exitCode() != 0)
    {
        // pdflatex render failed and we haven't pdf file
        result->successful = false;

        QString renderErrorText = QString::fromUtf8(p.readAllStandardOutput());
        renderErrorText.remove(0, renderErrorText.indexOf(QLatin1Char('!')));
        renderErrorText.remove(renderErrorText.indexOf(QLatin1String("!  ==> Fatal error occurred")), renderErrorText.size());
        renderErrorText = renderErrorText.trimmed();
        result->errorMessage = renderErrorText;

        finalize(result);
        texFile.setAutoRemove(false); //Useful for debug
        return;
    }

    //Clean up .aux and .log files
    QString pathWithoutExtension = tempDir + QDir::separator() + QLatin1String("cantor_")+uuid;
    QFile::remove(pathWithoutExtension + QLatin1String(".log"));
    QFile::remove(pathWithoutExtension + QLatin1String(".aux"));

    const QString& pdfFileName = pathWithoutExtension + QLatin1String(".pdf");

    bool success; QString errorMessage; QSizeF size;
    const auto& data = renderPdfToFormat(pdfFileName, m_code, uuid, m_type, m_scale, m_highResolution, &success, &errorMessage);
    QFile::remove(pdfFileName);
    result->successful = success;
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

std::pair<QTextImageFormat, QImage> MathRenderTask::renderPdfToFormat(const QString& filename, const QString& code, const QString uuid, Cantor::LatexRenderer::EquationType type, double scale, bool highResulution, bool* success, QString* errorReason)
{
    QSizeF size;
    const QImage& image = Cantor::Renderer::pdfRenderToImage(QUrl::fromLocalFile(filename), scale, highResulution, &size, errorReason);
    if (success)
        *success = image.isNull() == false;

    if (success && *success == false)
        return std::make_pair(QTextImageFormat(), QImage());

    QTextImageFormat format;

    QUrl internal;
    internal.setScheme(QLatin1String("internal"));
    internal.setPath(uuid);

    format.setName(internal.url());
    format.setWidth(size.width());
    format.setHeight(size.height());
    format.setProperty(Cantor::Renderer::CantorFormula, type);
    format.setProperty(Cantor::Renderer::ImagePath, filename);
    format.setProperty(Cantor::Renderer::Code, code);
    format.setVerticalAlignment(QTextCharFormat::AlignBaseline);

    switch(type)
    {
        case Cantor::LatexRenderer::FullEquation:
            format.setProperty(Cantor::Renderer::Delimiter, QLatin1String("$$"));
            break;

        case Cantor::LatexRenderer::InlineEquation:
            format.setProperty(Cantor::Renderer::Delimiter, QLatin1String("$"));
            break;

        case Cantor::LatexRenderer::CustomEquation:
            format.setProperty(Cantor::Renderer::Delimiter, QLatin1String(""));
            break;
    }

    return std::make_pair(std::move(format), std::move(image));
}
