/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
*/

#include <QApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>
#include <QTextStream>

#include "juliaserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    if (!QDBusConnection::sessionBus().isConnected()) {
        qWarning() << "Can't connect to the D-Bus session bus.\n"
                      "To start it, run: eval `dbus-launch --auto-syntax`";
        return 1;
    }

    const QString &serviceName =
        QString::fromLatin1("org.kde.Cantor.Julia-%1").arg(app.applicationPid());

    if (!QDBusConnection::sessionBus().registerService(serviceName)) {
        qWarning() << QDBusConnection::sessionBus().lastError().message();
        return 2;
    }

    JuliaServer server;
    QDBusConnection::sessionBus().registerObject(
        QLatin1String("/"),
        &server,
        QDBusConnection::ExportAllSlots
    );

    QTextStream(stdout) << "ready" << endl;

    return app.exec();
}
