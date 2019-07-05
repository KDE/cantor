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

#ifndef JUPYTERUTILS_H
#define JUPYTERUTILS_H

#include <vector>

#include <QString>
#include <QMimeDatabase>

class QJsonValue;
class QJsonObject;
class QJsonArray;
class QJsonDocument;
class QImage;
class QUrl;
class QStringList;

namespace Cantor {
    class Backend;
}

/**
 * Static class for storing some common code for working with jupyter json scheme
 * Like getting 'metadata', getting source code from 'source' tag, scheme validation
 * handleling missing keys, etc.
 *
 */
class JupyterUtils
{
  public:
    static QJsonObject getMetadata(const QJsonObject& object);
    static QJsonObject getCantorMetadata(const QJsonObject object);

    static QJsonArray getCells(const QJsonObject notebook);
    static std::tuple<int, int> getNbformatVersion(const QJsonObject& notebook);

    static QString getCellType(const QJsonObject& cell);
    static QString getSource(const QJsonObject& cell);
    static void setSource(QJsonObject& cell, const QString& source);

    static QString getOutputType(const QJsonObject& output);

    static bool isJupyterNotebook(const QJsonDocument& doc);

    static bool isJupyterCell(const QJsonValue& cell);
    static bool isMarkdownCell(const QJsonValue& cell);
    static bool isCodeCell(const QJsonValue& cell);
    static bool isRawCell(const QJsonValue& cell);

    static bool isJupyterOutput(const QJsonValue& output);
    static bool isJupyterDisplayOutput(const QJsonValue& output);
    static bool isJupyterTextOutput(const QJsonValue& output);
    static bool isJupyterErrorOutput(const QJsonValue& output);
    static bool isJupyterExecutionResult(const QJsonValue& output);

    static QJsonValue toJupyterMultiline(const QString& source);
    static QString fromJupyterMultiline(const QJsonValue& source);

    static QString getKernelName(const QJsonValue& kernelspecValue);
    static QJsonObject getKernelspec(const Cantor::Backend* backend);

    static QImage loadImage(const QJsonValue& mimeBundle, const QString& key);
    static QJsonObject packMimeBundle(const QImage& image, const QString& mime);
    static QStringList imageKeys(const QJsonValue& mimeBundle);
    static QString firstImageKey(const QJsonValue& mimeBundle);
    static QString mainBundleKey(const QJsonValue& mimeBundle);

    static bool isGifHtml(const QJsonValue& html);
    static QUrl loadGifHtml(const QJsonValue& html);
  public:
    static const QString cellsKey;
    static const QString metadataKey;
    static const QString cantorMetadataKey;
    static const QString nbformatKey;
    static const QString nbformatMinorKey;
    static const QString cellTypeKey;
    static const QString sourceKey;
    static const QString outputTypeKey;
    static const QString executionCountKey;
    static const QString outputsKey;
    static const QString dataKey;

    static const QString pngMime;
    static const QString gifMime;
    static const QString textMime;
    static const QString htmlMime;
    static const QString latexMime;

    static const QMimeDatabase mimeDatabase;
};

#endif // JUPYTERUTILS_H
