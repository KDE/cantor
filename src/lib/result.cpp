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

#include <QUrl>
#include <QJsonObject>
#include <QRegularExpression>

class Cantor::ResultPrivate
{
  public:
    ~ResultPrivate()
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
    html.replace(QRegularExpression(QStringLiteral("<br/>[\n]")), QStringLiteral("\n"));
    //remove all the unknown tags
    html.remove(QRegularExpression(QStringLiteral("<[a-zA-Z\\/][^>]*>") ));
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

int Cantor::Result::executionIndex() const
{
    return d->executionIndex;
}

void Cantor::Result::setExecutionIndex(int index)
{
    d->executionIndex = index;
}
