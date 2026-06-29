/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "result.h"
#include "jupyterutils.h"
using namespace Cantor;

#include <QUrl>
#include <QJsonObject>
#include <QRegularExpression>
#include <QUuid>

class Cantor::ResultPrivate
{
  public:
    ~ResultPrivate()
    {
        if (jupyterMetadata)
            delete jupyterMetadata;
    }

    QJsonObject* jupyterMetadata{nullptr};
    QString resultId;
    QString displayName;
    Result::Role role{Result::Role::Generic};
    int executionIndex{-1};
};

Result::Result() : d(new ResultPrivate)
{
    regenerateResultId();
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
    QJsonObject metadata = d->jupyterMetadata ? *d->jupyterMetadata : QJsonObject();
    QJsonObject cantorMetadata = metadata.value(JupyterUtils::cantorMetadataKey).toObject();

    cantorMetadata.insert(QLatin1String("result-id"), d->resultId);
    cantorMetadata.insert(QLatin1String("result-role"), roleToString(d->role));
    if (d->displayName.isEmpty())
        cantorMetadata.remove(QLatin1String("result-title"));
    else
        cantorMetadata.insert(QLatin1String("result-title"), d->displayName);

    metadata.insert(JupyterUtils::cantorMetadataKey, cantorMetadata);
    return metadata;
}

void Cantor::Result::setJupyterMetadata(const QJsonObject& metadata)
{
    if (!d->jupyterMetadata)
        d->jupyterMetadata = new QJsonObject();
    *d->jupyterMetadata = metadata;

    const QJsonObject cantorMetadata = metadata.value(JupyterUtils::cantorMetadataKey).toObject();
    const QString storedResultId = cantorMetadata.value(QLatin1String("result-id")).toString();
    if (!storedResultId.isEmpty())
        d->resultId = storedResultId;

    d->role = roleFromString(cantorMetadata.value(QLatin1String("result-role")).toString());

    const QJsonValue storedTitle = cantorMetadata.value(QLatin1String("result-title"));
    if (storedTitle.isString())
        d->displayName = storedTitle.toString().trimmed();
    else
        d->displayName.clear();
}

int Cantor::Result::executionIndex() const
{
    return d->executionIndex;
}

void Cantor::Result::setExecutionIndex(int index)
{
    d->executionIndex = index;
}

QString Cantor::Result::resultId() const
{
    return d->resultId;
}

void Cantor::Result::setResultId(const QString& id)
{
    if (id.isEmpty())
        regenerateResultId();
    else
        d->resultId = id;
}

void Cantor::Result::regenerateResultId()
{
    d->resultId = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString Cantor::Result::displayName() const
{
    return d->displayName;
}

void Cantor::Result::setDisplayName(const QString& name)
{
    d->displayName = name.trimmed();
}

Cantor::Result::Role Cantor::Result::role() const
{
    return d->role;
}

void Cantor::Result::setRole(Role role)
{
    d->role = role;
}

QString Cantor::Result::roleToString(Role role)
{
    switch (role)
    {
        case Role::Plot:
            return QStringLiteral("plot");
        case Role::Generic:
            return QStringLiteral("generic");
    }

    return QStringLiteral("generic");
}

Cantor::Result::Role Cantor::Result::roleFromString(const QString& roleName)
{
    if (roleName == QLatin1String("plot"))
        return Role::Plot;

    return Role::Generic;
}

void Cantor::Result::applyXmlResultMetadata(QDomElement& element) const
{
    element.setAttribute(QLatin1String("result-id"), d->resultId);
    element.setAttribute(QLatin1String("result-role"), roleToString(d->role));
    if (!d->displayName.isEmpty())
        element.setAttribute(QLatin1String("result-title"), d->displayName);
}

void Cantor::Result::loadXmlResultMetadata(const QDomElement& element)
{
    const QString storedResultId = element.attribute(QLatin1String("result-id"));
    if (!storedResultId.isEmpty())
        d->resultId = storedResultId;

    d->role = roleFromString(element.attribute(QLatin1String("result-role")));
    d->displayName = element.attribute(QLatin1String("result-title")).trimmed();
}
