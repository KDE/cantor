/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2015 Minh Ngo <minh@fedoraproject.org>
*/

#ifndef _PYTHONUTILS_H
#define _PYTHONUTILS_H

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QStandardPaths>

inline QString pythonServerExecutablePath()
{
#ifdef Q_OS_WIN
    const QString executableName = QStringLiteral("cantor_pythonserver.exe");
#else
    const QString executableName = QStringLiteral("cantor_pythonserver");
#endif
    const QString applicationServer = QDir(QCoreApplication::applicationDirPath()).filePath(executableName);
    if (QFileInfo(applicationServer).isExecutable())
        return applicationServer;

    return QStandardPaths::findExecutable(executableName);
}

inline QString fromSource(const QString& resourceName)
{
    QFile text(resourceName);
    if (text.open(QIODevice::ReadOnly))
        return QString::fromUtf8(text.readAll());
    else
    {
        qWarning() << "Cantor Python resource" << resourceName << "didn't open - something wrong";
        return QString();
    }
}

#endif
