/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
*/
#pragma once

#include <QFile>
#include <QStandardPaths>
#include <QDebug>

inline QString loadScript(const QString &scriptName)
{
    QString file = QStandardPaths::locate(
        QStandardPaths::AppDataLocation,
        QString::fromLatin1("juliabackend/scripts/%1.jl").arg(scriptName)
    );

    if (file.isEmpty())
        file = QStandardPaths::locate(
            QStandardPaths::GenericDataLocation,
            QString::fromLatin1("cantor/juliabackend/scripts/%1.jl").arg(scriptName)
        );

    QFile text(file);
    if (text.open(QIODevice::ReadOnly))
        return QString::fromUtf8(text.readAll());
    else
    {
        qWarning() << "Cantor Julia script" << scriptName+QLatin1String(".jl") << "not found - something wrong";
        return QString();
    }
}
