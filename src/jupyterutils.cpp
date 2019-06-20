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

#include "jupyterutils.h"

#include <tuple>

#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStringList>
#include <QSet>

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
            text.append(line);
        }
        return text;
    }
    else
        return QJsonValue(source);
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
    static const QSet<QString> notebookScheme
        = QSet<QString>::fromList({cellsKey, metadataKey, nbformatKey, nbformatMinorKey});

    bool isNotebook =
            doc.isObject()
        && QSet<QString>::fromList(doc.object().keys()) == notebookScheme
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
    return isJupyterCell(cell) && getCellType(cell.toObject()) == QLatin1String("code");
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
