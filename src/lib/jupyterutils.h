/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2019 Sirgienko Nikita <warquark@gmail.com>
*/

#ifndef JUPYTERUTILS_H
#define JUPYTERUTILS_H

#include <vector>

#include <QString>
#include <QMimeDatabase>

#include "cantor_export.h"

class QJsonValue;
class QJsonObject;
class QJsonArray;
class QJsonDocument;
class QImage;
class QUrl;
class QStringList;

namespace Cantor {

class Backend;

/**
 * Static class for storing some common code for working with jupyter json scheme
 * Like getting 'metadata', getting source code from 'source' tag, scheme validation
 * handleling missing keys, etc.
 *
 */
class CANTOR_EXPORT JupyterUtils
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

    /// Actually, this function handle only Jupyter notebooks version >= 4.0.0
    /// Previous versions treats as 'not notebook'
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
    static const QString svgMime;

    static const QMimeDatabase mimeDatabase;
};
}

#endif // JUPYTERUTILS_H
