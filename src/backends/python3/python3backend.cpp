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
 */

#include "python3backend.h"

#include <klocalizedstring.h>

#include "python3session.h"
#include "cantor_macros.h"

Python3Backend::Python3Backend(QObject* parent, const QList<QVariant> args)
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

KUrl Python3Backend::helpUrl() const
{
    return i18nc("the url to the documentation Python 3", "http://docs.python.org/3/");
}

QString Python3Backend::description() const
{
    return i18n("<p>Python is a remarkably powerful dynamic programming language that is used in a wide variety of application domains. " \
                "There are several Python packages to scientific programming.</p>" \
                "<p>This backend supports Python 3.</p>");
}

K_EXPORT_CANTOR_PLUGIN(python3backend, Python3Backend)

#include "python3backend.moc"