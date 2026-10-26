/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2018-2026 Alexander Semke <alexander.semke@web.de>
*/

#include "rserver.h"
#include <QCoreApplication>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    RServer server;
    return app.exec();
}
