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
    Copyright (C) 2014, 2015 Minh Ngo <minh@fedoraproject.org>
    Copyright (C) 2019 Alexander Semke <alexander.semke@web.de>
 */

#include "python3backend.h"
#include "python3session.h"
#include "settings.h"

#include <klocalizedstring.h>

Python3Backend::Python3Backend(QObject* parent, const QList<QVariant>& args)
    : PythonBackend(parent, args)
{
    setObjectName(QLatin1String("python3backend"));
}

Cantor::Session* Python3Backend::createSession()
{
    return new Python3Session(this);
}

QString Python3Backend::id() const
{
    return QLatin1String("python3");
}

QString Python3Backend::version() const
{
    return QLatin1String("3.6");
}

Cantor::Backend::Capabilities Python3Backend::capabilities() const
{
    qDebug()<<"Requesting capabilities of Python3Session";

    Backend::Capabilities cap =
        Cantor::Backend::SyntaxHighlighting |
        Cantor::Backend::Completion         |
        Cantor::Backend::SyntaxHelp;

    if(PythonSettings::variableManagement())
        cap |= Cantor::Backend::VariableManagement;

    return cap;
}

QUrl Python3Backend::helpUrl() const
{
    const QUrl& localDoc = PythonSettings::self()->localDoc();
    if (!localDoc.isEmpty())
        return localDoc;
    else
        return QUrl(i18nc("The url to the documentation Python 3", "https://docs.python.org/3/"));
}

QString Python3Backend::description() const
{
    return i18n("<b>Python</b> is a remarkably powerful dynamic programming language that is used in a wide variety of application domains. " \
                "There are several Python packages to scientific programming. " \
                "This backend supports Python 3.");
}

KConfigSkeleton* Python3Backend::config() const
{
    return PythonSettings::self();
}

bool Python3Backend::requirementsFullfilled(QString* const reason) const
{
    const QString& path = QStandardPaths::findExecutable(QLatin1String("cantor_python3server"));
    return Cantor::Backend::checkExecutable(QLatin1String("Cantor Python3 Server"), path, reason);
}

K_PLUGIN_FACTORY_WITH_JSON(python3backend, "python3backend.json", registerPlugin<Python3Backend>();)
#include "python3backend.moc"
