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

#include "result.h"
using namespace Cantor;

#include <QRegExp>
#include <QUrl>
#include <QJsonObject>

class Cantor::ResultPrivate
{
  public:
    ResultPrivate::~ResultPrivate()
    {
        if (jupyterMetadata)
            delete jupyterMetadata;
    }

    QJsonObject* jupyterMetadata{nullptr};
    int executionIndex{-1};
};


Result::Result() : d(new ResultPrivate)
{

}

Result::~Result()
{
    delete d;
}

QUrl Result::url()
{
    return QUrl();
}

QString Result::toLatex()
{
    QString html=toHtml();
    //replace linebreaks
    html.replace(QRegExp(QLatin1String("<br/>[\n]")), QStringLiteral("\n"));
    //remove all the unknown tags
    html.remove( QRegExp( QLatin1String("<[a-zA-Z\\/][^>]*>") ) );
    return QStringLiteral("\\begin{verbatim} %1 \\end{verbatim}").arg(html);
}

void Result::saveAdditionalData(KZip* archive)
{
    Q_UNUSED(archive)
    //Do nothing
}

QJsonObject Cantor::Result::jupyterMetadata() const
{
    return d->jupyterMetadata ? *d->jupyterMetadata : QJsonObject();
}

void Cantor::Result::setJupyterMetadata(QJsonObject metadata)
{
    if (!d->jupyterMetadata)
        d->jupyterMetadata = new QJsonObject();
    *d->jupyterMetadata = metadata;
}

QJsonArray Cantor::Result::toJupyterMultiline(const QString& source)
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

int Cantor::Result::executionIndex() const
{
    return d->executionIndex;
}

void Cantor::Result::setExecutionIndex(int index)
{
    d->executionIndex = index;
}
