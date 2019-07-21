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

#include "mathrender.h"

#include <QThreadPool>
#include <QDebug>
#include <QStandardPaths>
#include <QFile>

#include "mathrendertask.h"
#include "epsrenderer.h"

QMutex MathRenderer::popplerMutex;

MathRenderer::MathRenderer(): m_scale(1.0), m_useHighRes(false)
{
    qRegisterMetaType<QSharedPointer<MathRenderResult>>();
}

MathRenderer::~MathRenderer()
{
}

bool MathRenderer::mathRenderAvailable()
{
    return QStandardPaths::findExecutable(QLatin1String("pdflatex")).isEmpty() == false;
}

qreal MathRenderer::scale()
{
    return m_scale;
}

void MathRenderer::setScale(qreal scale)
{
    m_scale = scale;
}

void MathRenderer::useHighResolution(bool b)
{
    m_useHighRes = b;
}

void MathRenderer::renderExpression(const QString& mathExpression, Cantor::LatexRenderer::EquationType type, const QObject* receiver, const char* resultHandler)
{
    MathRenderTask* task = new MathRenderTask(mathExpression, type, m_scale, m_useHighRes, &popplerMutex);
    task->setHandler(receiver, resultHandler);
    task->setAutoDelete(false);

    QThreadPool::globalInstance()->start(task);
}

void MathRenderer::rerender(QTextDocument* document, const QTextImageFormat& math)
{
    const QString& filename = math.property(EpsRenderer::ImagePath).toString();
    if (!QFile::exists(filename))
        return;

    bool success; QString errorMessage;
    QImage img = MathRenderTask::renderPdf(filename, m_scale, m_useHighRes, &success, nullptr, &errorMessage, &popplerMutex);

    if (success)
    {
        QUrl internal(math.name());
        document->addResource(QTextDocument::ImageResource, internal, QVariant(img));
    }
    else
    {
        qDebug() << "Rerender embedded math failed with message: " << errorMessage;
    }
}

std::pair<QTextImageFormat, QImage> MathRenderer::renderExpressionFromPdf(const QString& filename, const QString& uuid, const QString& code, Cantor::LatexRenderer::EquationType type, bool* outSuccess)
{
    if (!QFile::exists(filename))
    {
        if (outSuccess)
            *outSuccess = false;
        return std::make_pair(QTextImageFormat(), QImage());
    }

    bool success; QString errorMessage;
    const auto& data = MathRenderTask::renderPdfToFormat(filename, code, uuid, type, m_scale, m_useHighRes, &success, &errorMessage, &popplerMutex);
    if (success == false)
        qDebug() << "Render embedded math from pdf failed with message: " << errorMessage;
    if (outSuccess)
        *outSuccess = success;
    return data;
}
