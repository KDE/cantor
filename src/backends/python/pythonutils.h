/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2015 Minh Ngo <minh@fedoraproject.org>
*/

#ifndef _PYTHONUTILS_H
#define _PYTHONUTILS_H

#include <QFile>
#include <QDebug>

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
