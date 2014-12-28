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
    Copyright (C) 2014 Minh Ngo <minh@fedoraproject.org>
 */

#include "python2backend.h"

#include <klocalizedstring.h>

#include "cantor_macros.h"

Python2Backend::Python2Backend(QObject* parent, const QList<QVariant> args)
    : PythonBackend(parent, args)
{
    setObjectName(QLatin1String("python2backend"));
    // Because the plugin may not have been loaded with
    // ExportExternalSymbols, we load the python symbols again
    // to make sure that python modules such as numpy see them
    // (see bug #330032)
    QLibrary pythonLib(QLatin1String("python2.7"));
    pythonLib.setLoadHints(QLibrary::ExportExternalSymbolsHint);
    pythonLib.load();
}

QString Python2Backend::id() const
{
    return QLatin1String("python2");
}

KUrl Python2Backend::helpUrl() const
{
    return i18nc("the url to the documentation Python 2", "http://docs.python.org/2/");
}

QString Python2Backend::description() const
{
    return i18n("<p>Python is a remarkably powerful dynamic programming language that is used in a wide variety of application domains. " \
                "There are several Python packages to scientific programming.</p>" \
                "<p>This backend supports Python 2.</p>");
}

K_EXPORT_CANTOR_PLUGIN(python2backend, Python2Backend)

#include "python2backend.moc"