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
    Copyright (C) 2015 Minh Ngo <minh@fedoraproject.org>
 */

#include <iostream>
#include <csignal>

#include <QApplication>
#include <QTimer>
#include <QChar>
#include <QByteArray>

#include "pythonserver.h"

const QChar recordSep(30);
const QChar unitSep(31);
const char messageEnd = 29;

PythonServer server;
bool isInterrupted = false;
QTimer inputTimer;
QMetaObject::Connection connection;

QString inputBuffer;

QLatin1String LOGIN("login");
QLatin1String EXIT("exit");
QLatin1String CODE("code");
QLatin1String FILEPATH("setFilePath");
QLatin1String MODEL("model");

void routeInput() {
    QByteArray bytes;
    char c;
    while (std::cin.get(c))
    {
        if (messageEnd == c)
            break;
        else
            bytes.append(c);
    }
    inputBuffer.append(QString::fromLocal8Bit(bytes));
    if (inputBuffer.isEmpty())
        return;

    const QStringList& records = inputBuffer.split(recordSep);
    inputBuffer.clear();

    if (records.size() == 2)
    {
        if (records[0] == EXIT)
        {
            QObject::disconnect(connection);
            QObject::connect(&inputTimer, &QTimer::timeout, QCoreApplication::instance(), &QCoreApplication::quit);
        }
        else if (records[0] == LOGIN)
        {
            server.login();
        }
        else if (records[0] == CODE)
        {
            server.runPythonCommand(records[1]);

            if (!isInterrupted)
            {
                const QString& result =
                    server.getOutput()
                    + unitSep
                    + server.getError()
                    + QLatin1Char(messageEnd);

                const QByteArray bytes = result.toLocal8Bit();
                std::cout << bytes.data();
            }
            else
            {
                // No replay when interrupted
                isInterrupted = false;
            }
        }
        else if (records[0] == FILEPATH)
        {
            server.setFilePath(records[1]);
        }
        else if (records[0] == MODEL)
        {
            bool ok;
            bool val = records[1].toInt(&ok);

            QString result;
            if (ok)
                result = server.variables(val) + unitSep;
            else
                result = unitSep + QLatin1String("Invalid argument %1 for 'model' command", val);
            result += QLatin1Char(messageEnd);

            const QByteArray bytes = result.toLocal8Bit();
            std::cout << bytes.data();
        }
        std::cout.flush();
    }
}

void signal_handler(int signal)
{
    if (signal == SIGINT)
    {
        isInterrupted = true;
        server.interrupt();
    }
}


int main(int argc, char *argv[])
{
    std::signal(SIGINT, signal_handler);
    QCoreApplication app(argc, argv);

    connection = QObject::connect(&inputTimer, &QTimer::timeout, routeInput);
    inputTimer.setInterval(100);
    inputTimer.start();

    std::cout << "ready" << std::endl;

    return app.exec();
}
