#ifndef RUTILS_H
#define RUTILS_H

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

inline QString rServerExecutablePath()
{
#ifdef Q_OS_WIN
    const QString executableName = QStringLiteral("cantor_rserver.exe");
#else
    const QString executableName = QStringLiteral("cantor_rserver");
#endif
    const QString applicationServer = QDir(QCoreApplication::applicationDirPath()).filePath(executableName);
    if (QFileInfo(applicationServer).isExecutable())
        return applicationServer;

    return QStandardPaths::findExecutable(executableName);
}

#endif
