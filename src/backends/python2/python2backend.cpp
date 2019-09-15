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

#include "python2backend.h"
#include "python2session.h"
#include "settings.h"

#include <KLocalizedString>

Python2Backend::Python2Backend(QObject* parent, const QList<QVariant> args)
    : PythonBackend(parent, args)
{
    // Because the plugin may not have been loaded with
    // ExportExternalSymbols, we load the python symbols again
    // to make sure that python modules such as numpy see them
    // (see bug #330032)
    QLibrary pythonLib(QLatin1String("python2.7"));
    pythonLib.setLoadHints(QLibrary::ExportExternalSymbolsHint);
    pythonLib.load();
}

Cantor::Session* Python2Backend::createSession()
{
    return new Python2Session(this);
}

QString Python2Backend::id() const
{
    return QLatin1String("python2");
}

QString Python2Backend::version() const
{
    return QLatin1String("2.7");
}

Cantor::Backend::Capabilities Python2Backend::capabilities() const
{
    Backend::Capabilities cap =
        Cantor::Backend::SyntaxHighlighting |
        Cantor::Backend::Completion         |
        Cantor::Backend::SyntaxHelp;

    if(PythonSettings::variableManagement())
        cap |= Cantor::Backend::VariableManagement;

    return cap;
}

QUrl Python2Backend::helpUrl() const
{
    const QUrl& localDoc = PythonSettings::self()->localDoc();
    if (!localDoc.isEmpty())
        return localDoc;
    else
        return QUrl(i18nc("The url to the documentation Python 2", "https://docs.python.org/2/"));
}

QString Python2Backend::description() const
{
    return i18n("<b>Python</b> is a remarkably powerful dynamic programming language that is used in a wide variety of application domains. " \
                "There are several Python packages to scientific programming. " \
                "This backend supports Python 2.");
}

KConfigSkeleton* Python2Backend::config() const
{
    return PythonSettings::self();
}

bool Python2Backend::requirementsFullfilled(QString* const reason) const
{
    const QString& path = QStandardPaths::findExecutable(QLatin1String("cantor_python2server"));
    return Cantor::Backend::checkExecutable(QLatin1String("Cantor Python2 Server"), path, reason);
}

K_PLUGIN_FACTORY_WITH_JSON(python2backend, "python2backend.json", registerPlugin<Python2Backend>();)
#include "python2backend.moc"
