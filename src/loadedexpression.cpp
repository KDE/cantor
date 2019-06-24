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

#include "loadedexpression.h"

#include "jupyterutils.h"
#include "lib/imageresult.h"
#include "lib/epsresult.h"
#include "lib/textresult.h"
#include "lib/latexresult.h"
#include "lib/animationresult.h"

#include <QDir>
#include <QStandardPaths>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>
#include <QPixmap>
#include <QTemporaryFile>

LoadedExpression::LoadedExpression( Cantor::Session* session ) : Cantor::Expression( session, false, -1)
{

}

void LoadedExpression::interrupt()
{
    //Do nothing
}

void LoadedExpression::evaluate()
{
    //Do nothing
}

void LoadedExpression::loadFromXml(const QDomElement& xml, const KZip& file)
{
    setCommand(xml.firstChildElement(QLatin1String("Command")).text());

    const QDomNodeList& results = xml.elementsByTagName(QLatin1String("Result"));
    for (int i = 0; i < results.size(); i++)
    {
        const QDomElement& resultElement = results.at(i).toElement();
        const QString& type = resultElement.attribute(QLatin1String("type"));
        if ( type == QLatin1String("text"))
        {
            addResult(new Cantor::TextResult(resultElement.text()));
        }
        else if (type == QLatin1String("image") || type == QLatin1String("latex") || type == QLatin1String("animation"))
        {
            const KArchiveEntry* imageEntry=file.directory()->entry(resultElement.attribute(QLatin1String("filename")));
            if (imageEntry&&imageEntry->isFile())
            {
                const KArchiveFile* imageFile=static_cast<const KArchiveFile*>(imageEntry);
                QString dir=QStandardPaths::writableLocation(QStandardPaths::TempLocation);
                imageFile->copyTo(dir);
                QUrl imageUrl = QUrl::fromLocalFile(QDir(dir).absoluteFilePath(imageFile->name()));
                if(type==QLatin1String("latex"))
                {
                    addResult(new Cantor::LatexResult(resultElement.text(), imageUrl));
                }else if(type==QLatin1String("animation"))
                {
                    addResult(new Cantor::AnimationResult(imageUrl));
                }else if(imageFile->name().endsWith(QLatin1String(".eps")))
                {
                    addResult(new Cantor::EpsResult(imageUrl));
                }else
                {
                    addResult(new Cantor::ImageResult(imageUrl));
                }
            }
        }
    }

    const QDomElement& errElem = xml.firstChildElement(QLatin1String("Error"));
    if (!errElem.isNull())
    {
        setErrorMessage(errElem.text());
        setStatus(Error);
    }
    else
        setStatus(Done);
}

void LoadedExpression::loadFromJupyter(const QJsonObject& cell)
{
    setCommand(JupyterUtils::getSource(cell));

    const QJsonValue idObject = cell.value(QLatin1String("execution_count"));
    if (!idObject.isUndefined() && !idObject.isNull())
        setId(idObject.toInt());

    const QJsonArray& outputs = cell.value(QLatin1String("outputs")).toArray();
    for (QJsonArray::const_iterator iter = outputs.begin(); iter != outputs.end(); iter++)
    {
        if (!JupyterUtils::isJupyterOutput(*iter))
            continue;

        const QJsonObject& output = iter->toObject();
        const QString& outputType = JupyterUtils::getOutputType(output);
        if (outputType == QLatin1String("stream"))
        {
            const QString& text = JupyterUtils::fromJupyterMultiline(output.value(QLatin1String("text")));

            addResult(new Cantor::TextResult(text));
        }
        else if (outputType == QLatin1String("error"))
        {
            const QJsonArray& tracebackLineArray = output.value(QLatin1String("traceback")).toArray();
            QString traceback;

            // Looks like the traceback in Jupyter joined with '\n', no ''
            // So, manually add it
            for (const QJsonValue& line : tracebackLineArray)
                traceback += line.toString() + QLatin1Char('\n');
            traceback.chop(1);

            // IPython returns error with terminal colors, we handle it here, but should we?
            static const QChar ESC(0x1b);
            traceback.remove(QRegExp(QString(ESC)+QLatin1String("\\[[0-9;]*m")));

            setErrorMessage(traceback.toHtmlEscaped().replace(QLatin1String("\n"), QLatin1String("<br>")));
        }
        else if (outputType == QLatin1String("display_data") || outputType == QLatin1String("execute_result"))
        {
            // Jupyter TODO: handle plain/html?
            const QJsonObject& data = output.value(QLatin1String("data")).toObject();

            const QString& text = JupyterUtils::fromJupyterMultiline(data.value(JupyterUtils::textMime));
            const QString& imageKey = JupyterUtils::firstImageKey(data);
            if (!imageKey.isEmpty())
            {
                if (imageKey == QLatin1String("image/gif"))
                {
                    const QByteArray& bytes = QByteArray::fromBase64(data.value(imageKey).toString().toLatin1());

                    QTemporaryFile file;
                    file.setAutoRemove(false);
                    file.open();
                    file.write(bytes);
                    file.close();

                    addResult(new Cantor::AnimationResult(QUrl::fromLocalFile(file.fileName()), text));
                }
                else
                {
                    QImage image = JupyterUtils::loadImage(data, imageKey);

                    const QJsonObject& metadata = JupyterUtils::getMetadata(output);
                    const QJsonValue size = metadata.value(JupyterUtils::pngMime);
                    if (size.isObject())
                    {
                        int w = size.toObject().value(QLatin1String("width")).toInt();
                        int h = size.toObject().value(QLatin1String("height")).toInt();

                        if (w != 0 && h != 0)
                            image = image.scaled(w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                    }

                    addResult(new Cantor::ImageResult(image, text));
                }
            }
            else if (!text.isEmpty())
            {
                addResult(new Cantor::TextResult(text));
            }
        }
    }

    if (errorMessage().isEmpty())
        setStatus(Done);
    else
        setStatus(Error);
}
