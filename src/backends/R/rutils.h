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

    const QString path = QStandardPaths::findExecutable(executableName);
#ifdef Q_OS_MACOS
    if (path.isEmpty())
    {
        QDir appDir(QCoreApplication::applicationDirPath());
        appDir.cdUp(); appDir.cdUp(); appDir.cdUp(); // MacOS -> Contents -> cantor.app -> bin
        const QString bundlePath = appDir.filePath(QStringLiteral("cantor_rserver.app/Contents/MacOS/cantor_rserver"));
        if (QFileInfo(bundlePath).isExecutable())
            return bundlePath;
    }
#endif
    return path;
}

#endif
