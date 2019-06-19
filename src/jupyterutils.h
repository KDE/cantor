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

#include <QString>

class QJsonValue;
class QJsonObject;
class QJsonArray;
class QJsonDocument;

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

    static QJsonArray getCells(const QJsonObject notebook);
    static std::tuple<int, int> getNbformatVersion(const QJsonObject& notebook);

    static QString getCellType(const QJsonObject& cell);
    static QString getSource(const QJsonObject& cell);
    static void setSource(QJsonObject& cell, const QString& source);

    static QString getOutputType(const QJsonObject& output);

    static bool isJupyterNotebook(const QJsonDocument& doc);
    static bool isJupyterCell(const QJsonValue& cell);
    static bool isJupyterOutput(const QJsonValue& cell);
    static bool isMarkdownCell(const QJsonValue& cell);
    static bool isCodeCell(const QJsonValue& cell);
    static bool isRawCell(const QJsonValue& cell);

    static QJsonValue toJupyterMultiline(const QString& source);
    static QString fromJupyterMultiline(const QJsonValue& source);

  public:
    static const QString cellsKey;
    static const QString metadataKey;
    static const QString nbformatKey;
    static const QString nbformatMinorKey;
    static const QString cellTypeKey;
    static const QString sourceKey;
    static const QString outputTypeKey;
};

#endif // JUPYTERUTILS_H
