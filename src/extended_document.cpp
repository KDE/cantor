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

#include "extended_document.h"
#include "worksheettextitem.h"

QNetworkAccessManager ExtendedDocument::networkManager;

ExtendedDocument::ExtendedDocument(QObject *parent): QTextDocument(parent)
{

}

void ExtendedDocument::handleLoad(QNetworkReply* reply)
{
    const QUrl& requestUrl = reply->request().url();

    if (loading.contains(requestUrl))
    {
        if (reply->error() == QNetworkReply::NoError)
        {
            QImage img;
            img.loadFromData(reply->readAll());

            if (!img.isNull())
            {
                this->addResource(QTextDocument::ImageResource, reply->request().url(), QVariant(img));

                // TODO: find another way to redraw document
                QTextCursor cursor(this);
                cursor.movePosition(QTextCursor::End);
                cursor.insertText(QLatin1String("\n"));
                cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
                cursor.removeSelectedText();
            }
            else
                qDebug() << "content of url" << requestUrl << "not a image";
        }
        else
        {
            qDebug() << "loading image in document from" << requestUrl << "failed with error: " << reply->errorString();
        }

        loading.removeOne(requestUrl);
        if (loading.size() == 0)
            disconnect(&networkManager, &QNetworkAccessManager::finished, this, &ExtendedDocument::handleLoad);
    }
}

QVariant ExtendedDocument::loadResource(int type, const QUrl &name)
{
    if (type == QTextDocument::ImageResource && (name.scheme() == QLatin1String("http") || name.scheme() == QLatin1String("https")))
    {
        if (!loading.contains(name))
        {
            if (loading.size() == 0)
                connect(&networkManager, &QNetworkAccessManager::finished, this, &ExtendedDocument::handleLoad);

            QNetworkRequest request(name);
            request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
            networkManager.get(request);

            loading << name;
        }
        return QVariant();
    }
    else
        return QTextDocument::loadResource(type, name);
}
