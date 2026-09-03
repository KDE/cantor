#ifndef JULIAUTILS_H
#define JULIAUTILS_H

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

inline QString juliaServerExecutablePath()
{
#ifdef Q_OS_WIN
    const QString executableName = QStringLiteral("cantor_juliaserver.exe");
#else
    const QString executableName = QStringLiteral("cantor_juliaserver");
#endif
    const QString applicationServer = QDir(QCoreApplication::applicationDirPath()).filePath(executableName);
    if (QFileInfo(applicationServer).isExecutable())
        return applicationServer;

    return QStandardPaths::findExecutable(executableName);
}

#endif
