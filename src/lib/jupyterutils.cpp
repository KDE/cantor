/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2019 Sirgienko Nikita <warquark@gmail.com>
*/

#include "jupyterutils.h"
#include "backend.h"

#include <tuple>

#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSet>
#include <QImageReader>
#include <QImageWriter>
#include <QBuffer>
#include <QString>
#include <QUrl>
#include <QTemporaryFile>

using namespace Cantor;

const QString JupyterUtils::cellsKey = QLatin1String("cells");
const QString JupyterUtils::metadataKey = QLatin1String("metadata");
const QString JupyterUtils::cantorMetadataKey = QLatin1String("cantor");
const QString JupyterUtils::nbformatKey = QLatin1String("nbformat");
const QString JupyterUtils::nbformatMinorKey = QLatin1String("nbformat_minor");
const QString JupyterUtils::cellTypeKey = QLatin1String("cell_type");
const QString JupyterUtils::sourceKey = QLatin1String("source");
const QString JupyterUtils::outputTypeKey = QLatin1String("output_type");
const QString JupyterUtils::executionCountKey = QLatin1String("execution_count");
const QString JupyterUtils::outputsKey = QLatin1String("outputs");
const QString JupyterUtils::dataKey = QLatin1String("data");

const QString JupyterUtils::pngMime = QLatin1String("image/png");
const QString JupyterUtils::gifMime = QLatin1String("image/gif");
const QString JupyterUtils::textMime = QLatin1String("text/plain");
const QString JupyterUtils::htmlMime = QLatin1String("text/html");
const QString JupyterUtils::latexMime = QLatin1String("text/latex");
const QString JupyterUtils::svgMime = QLatin1String("image/svg+xml");

const QMimeDatabase JupyterUtils::mimeDatabase;

QJsonValue JupyterUtils::toJupyterMultiline(const QString& source)
{
    if (source.contains(QLatin1Char('\n')))
    {
        QJsonArray text;
        const QStringList& lines = source.split(QLatin1Char('\n'));
        for (int i = 0; i < lines.size(); i++)
        {
            QString line = lines[i];
            // Don't add \n to last line
            if (i != lines.size() - 1)
                line.append(QLatin1Char('\n'));
            // Ignore last line, if it is an empty line
            else if (line.isEmpty())
                break;

            text.append(line);
        }
        return text;
    }
    else
        return QJsonArray::fromStringList(QStringList(source));
}

QString JupyterUtils::fromJupyterMultiline(const QJsonValue& source)
{
    QString code;
    if (source.isString())
        code = source.toString();
    else if (source.isArray())
        for (const QJsonValue& line : source.toArray())
            code += line.toString();
    return code;
}

bool JupyterUtils::isJupyterNotebook(const QJsonDocument& doc)
{
    bool isNotebook = doc.isObject()
        && doc.object().keys().length() == 4
        && doc.object().value(cellsKey).isArray()
        && doc.object().value(metadataKey).isObject()
        && doc.object().value(nbformatKey).isDouble()
        && doc.object().value(nbformatMinorKey).isDouble();

    return isNotebook;
}

bool JupyterUtils::isJupyterCell(const QJsonValue& cell)
{
    bool isCell =
           cell.isObject()
        && cell.toObject().value(cellTypeKey).isString()
        &&
        (    cell.toObject().value(cellTypeKey).toString() == QLatin1String("markdown")
          || cell.toObject().value(cellTypeKey).toString() == QLatin1String("code")
          || cell.toObject().value(cellTypeKey).toString() == QLatin1String("raw")
        )
        && cell.toObject().value(metadataKey).isObject()
        &&
        (    cell.toObject().value(sourceKey).isString()
          || cell.toObject().value(sourceKey).isArray()
        );

    return isCell;
}

bool JupyterUtils::isJupyterOutput(const QJsonValue& output)
{
    bool isOutput =
           output.isObject()
        && output.toObject().value(outputTypeKey).isString()
        &&
        (    output.toObject().value(outputTypeKey).toString() == QLatin1String("stream")
          || output.toObject().value(outputTypeKey).toString() == QLatin1String("display_data")
          || output.toObject().value(outputTypeKey).toString() == QLatin1String("execute_result")
          || output.toObject().value(outputTypeKey).toString() == QLatin1String("error")
        );

    return isOutput;
}

bool JupyterUtils::isJupyterTextOutput(const QJsonValue& output)
{
    return
           isJupyterOutput(output)
        && output.toObject().value(outputTypeKey).toString() == QLatin1String("stream")
        && output.toObject().value(QLatin1String("name")).isString()
        && output.toObject().value(QLatin1String("text")).isArray();
}

bool JupyterUtils::isJupyterErrorOutput(const QJsonValue& output)
{
    return
           isJupyterOutput(output)
        && output.toObject().value(outputTypeKey).toString() == QLatin1String("error")
        && output.toObject().value(QLatin1String("ename")).isString()
        && output.toObject().value(QLatin1String("evalue")).isString()
        && output.toObject().value(QLatin1String("traceback")).isArray();
}

bool JupyterUtils::isJupyterExecutionResult(const QJsonValue& output)
{
    return
           isJupyterOutput(output)
        && output.toObject().value(outputTypeKey).toString() == QLatin1String("execute_result")
        && output.toObject().value(QLatin1String("execution_count")).isDouble()
        && output.toObject().value(metadataKey).isObject()
        && output.toObject().value(QLatin1String("data")).isObject();
}

bool JupyterUtils::isJupyterDisplayOutput(const QJsonValue& output)
{
    return
           isJupyterOutput(output)
        && output.toObject().value(outputTypeKey).toString() == QLatin1String("display_data")
        && output.toObject().value(metadataKey).isObject()
        && output.toObject().value(QLatin1String("data")).isObject();
}

bool JupyterUtils::isMarkdownCell(const QJsonValue& cell)
{
    return isJupyterCell(cell) && getCellType(cell.toObject()) == QLatin1String("markdown");
}

bool JupyterUtils::isCodeCell(const QJsonValue& cell)
{
    return
           isJupyterCell(cell)
        && getCellType(cell.toObject()) == QLatin1String("code")
        &&
        (      cell.toObject().value(executionCountKey).isDouble()
            || cell.toObject().value(executionCountKey).isNull()
        )
        && cell.toObject().value(outputsKey).isArray();
}

bool JupyterUtils::isRawCell(const QJsonValue& cell)
{
    return isJupyterCell(cell) && getCellType(cell.toObject()) == QLatin1String("raw");
}

QJsonObject JupyterUtils::getMetadata(const QJsonObject& object)
{
    return object.value(metadataKey).toObject();
}

QJsonArray JupyterUtils::getCells(const QJsonObject notebook)
{
    return notebook.value(cellsKey).toArray();
}

std::tuple<int, int> JupyterUtils::getNbformatVersion(const QJsonObject& notebook)
{
    int nbformatMajor = notebook.value(nbformatKey).toInt();
    int nbformatMinor = notebook.value(nbformatMinorKey).toInt();

    return {nbformatMajor, nbformatMinor};
}

QString JupyterUtils::getCellType(const QJsonObject& cell)
{
    return cell.value(cellTypeKey).toString();
}

QString JupyterUtils::getSource(const QJsonObject& cell)
{
    return fromJupyterMultiline(cell.value(sourceKey));
}

void JupyterUtils::setSource(QJsonObject& cell, const QString& source)
{
    cell.insert(sourceKey, toJupyterMultiline(source));
}

QString JupyterUtils::getOutputType(const QJsonObject& output)
{
    return output.value(outputTypeKey).toString();
}

QJsonObject JupyterUtils::getCantorMetadata(const QJsonObject object)
{
    return getMetadata(object).value(cantorMetadataKey).toObject();
}

QString JupyterUtils::getKernelName(const QJsonValue& kernelspecValue)
{
    QString name;

    if (kernelspecValue.isObject())
    {
        const QJsonObject& kernelspec = kernelspecValue.toObject();
        QString kernelName = kernelspec.value(QLatin1String("name")).toString();
        if (!kernelName.isEmpty())
        {
            if (kernelName.startsWith(QLatin1String("julia")))
                kernelName = QLatin1String("julia");
            else if (kernelName == QLatin1String("sagemath"))
                kernelName = QLatin1String("sage");
            else if (kernelName == QLatin1String("ir"))
                kernelName = QLatin1String("r");
            name = kernelName;
        }
        else
        {
            name = kernelspec.value(QLatin1String("language")).toString();
        }
    }

    return name;
}

QJsonObject JupyterUtils::getKernelspec(const Cantor::Backend* backend)
{
    QJsonObject kernelspec;

    if (backend)
    {
        QString id = backend->id();

        if (id == QLatin1String("sage"))
            id = QLatin1String("sagemath");
        else if (id == QLatin1String("r"))
            id = QLatin1String("ir");

        kernelspec.insert(QLatin1String("name"), id);

        QString lang = backend->id();
        if (lang.startsWith(QLatin1String("python")))
            lang = QLatin1String("python");
        lang[0] = lang[0].toUpper();

        kernelspec.insert(QLatin1String("language"), lang);

        kernelspec.insert(QLatin1String("display_name"), backend->name());
    }

    return kernelspec;
}

QImage JupyterUtils::loadImage(const QJsonValue& mimeBundle, const QString& key)
{
    QImage image;

    if (mimeBundle.isObject())
    {
        const QJsonObject& bundleObject = mimeBundle.toObject();
        const QJsonValue& data = bundleObject.value(key);
        if (data.isString() || data.isArray())
        {
            // In jupyter mime-bundle key for data is mime type of this data
            // So we need convert mimetype to format, for example "image/png" to "png"
            // for loading from data
            if (QImageReader::supportedMimeTypes().contains(key.toLatin1()))
            {
                const QByteArray& format = mimeDatabase.mimeTypeForName(key).preferredSuffix().toLatin1();
                // Handle svg separately, because Jupyter don't encode svg in base64
                // and store as jupyter multiline text
                if (key == QLatin1String("image/svg+xml") && data.isArray())
                {
                    image.loadFromData(fromJupyterMultiline(data).toLatin1(), format.data());
                }
                else if (data.isString())
                {
                    // https://doc.qt.io/qt-5/qimagereader.html#supportedImageFormats
                    // Maybe there is a better way to convert image key to image format
                    // but this is all that I could to do
                    const QString& base64 = data.toString();
                    image.loadFromData(QByteArray::fromBase64(base64.toLatin1()), format.data());
                }
            }
        }
    }

    return image;
}

QJsonObject JupyterUtils::packMimeBundle(const QImage& image, const QString& mime)
{
    QJsonObject mimeBundle;

    if (QImageWriter::supportedMimeTypes().contains(mime.toLatin1()))
    {
        const QByteArray& format = mimeDatabase.mimeTypeForName(mime).preferredSuffix().toLatin1();

        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, format.data());
        mimeBundle.insert(mime, QString::fromLatin1(ba.toBase64()));
    }

    return mimeBundle;
}

QStringList JupyterUtils::imageKeys(const QJsonValue& mimeBundle)
{
    QStringList imageKeys;

    if (mimeBundle.isObject())
    {
        const QStringList& keys = mimeBundle.toObject().keys();
        const QList<QByteArray>& mimes = QImageReader::supportedMimeTypes();
        for (const QString& key : keys)
            if (mimes.contains(key.toLatin1()))
                imageKeys.append(key);
    }

    return imageKeys;
}

QString JupyterUtils::firstImageKey(const QJsonValue& mimeBundle)
{
    const QStringList& keys = imageKeys(mimeBundle);
    return keys.size() >= 1 ? keys[0] : QString();
}

QString JupyterUtils::mainBundleKey(const QJsonValue& mimeBundle)
{
    QString mainKey;

    if (mimeBundle.isObject())
    {
        const QStringList& keys = mimeBundle.toObject().keys();
        if (keys.size() == 1)
            mainKey = keys[0];
        else if (keys.size() == 2)
        {
            int idx = keys.indexOf(textMime);
            if (idx != -1)
                // Getting not 'text/plain' key, because often it's just a caption
                mainKey = keys[1 - idx];
            else
                // Not sure, that this is valid, but return first keys
                mainKey = keys[0];
        }
        else if (keys.size() > 2)
        {
            // Also not sure about it
            // Specification is not very clean on cases, such that
            // Just in case, if we will have duplications of information
            // Something like keys == {'image/png', 'image/bmp', 'text/plain'}
            // Or something like keys == {'text/html', 'text/latex', 'text/plain'}
            // Set priority for html->latex->plain (in this order)
            if (keys.contains(htmlMime))
                mainKey = htmlMime;
            else if (keys.contains(latexMime))
                mainKey = latexMime;
            else if (keys.contains(textMime))
                mainKey = textMime;
            else
            {
                // Search for image keys, if no
                // then just use first key
                mainKey = firstImageKey(mimeBundle);
                if (mainKey.isEmpty())
                    mainKey = keys[0];
            }
        }
    }

    return mainKey;
}

bool JupyterUtils::isGifHtml(const QJsonValue& html)
{
    return html.isString()
        && html.toString().startsWith(QLatin1String("<img src=\"data:image/gif;base64,"))
        && html.toString().endsWith(QLatin1String("/>"));
}

QUrl JupyterUtils::loadGifHtml(const QJsonValue& html)
{
    QUrl gif;

    if (html.isString())
    {
        QString data = html.toString();
        data.remove(0, QString::fromLatin1("<img src=\"data:image/gif;base64,").size());
        data.chop(QString::fromLatin1("/>").size());
        const QByteArray& bytes = QByteArray::fromBase64(data.toLatin1());

        QTemporaryFile file;
        file.setAutoRemove(false);
        file.open();
        file.write(bytes);
        file.close();

        gif = QUrl::fromLocalFile(file.fileName());
    }

    return gif;
}
