/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
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
