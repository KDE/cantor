/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2018 Alexander Semke <alexander.semke@web.de>
*/

#include "rserver.h"

#include <QApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>
#include <QTextStream>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    if (!QDBusConnection::sessionBus().isConnected()) {
        qWarning() << "Can't connect to the D-Bus session bus.\n"
                      "To start it, run: eval `dbus-launch --auto-syntax`";
        return 1;
    }

    const QString &serviceName =
        QString::fromLatin1("org.kde.Cantor.R-%1").arg(app.applicationPid());

    if (!QDBusConnection::sessionBus().registerService(serviceName)) {
        qWarning() << QDBusConnection::sessionBus().lastError().message();
        return 2;
    }

    RServer server;
    QDBusConnection::sessionBus().registerObject(
        QLatin1String("/"),
        &server
    );

    return app.exec();
}
