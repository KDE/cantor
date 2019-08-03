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

#ifndef EXTENDEDDOCUMENT_H
#define EXTENDEDDOCUMENT_H

#include <QTextDocument>
#include <QDebug>
#include <QNetworkReply>
#include <QImage>
#include <QList>
#include <QGraphicsTextItem>
#include <QTextCursor>

/**
 * Additional class with one purpose - expand QTextDocument and
 * allow Image Resources from web (http and https urls)
 */
class ExtendedDocument : public QTextDocument
{
  public:
    ExtendedDocument(QObject *parent = nullptr);

  protected:
    QVariant loadResource(int type, const QUrl &name) override;

  private Q_SLOTS:
    void handleLoad(QNetworkReply *reply);

  private:
    QList<QUrl> loading; // List of currently loaded urls
    static QNetworkAccessManager networkManager;
};

#endif /* EXTENDEDDOCUMENT_H */
