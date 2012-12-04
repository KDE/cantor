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
    Copyright (C) 2012 Filipe Saraiva <filipe@kde.org>
 */

#include "pythonbackend.h"

#include "pythonsession.h"
#include "settings.h"
#include "ui_settings.h"

#include "kdebug.h"
#include <QWidget>

#include "cantor_macros.h"

PythonBackend::PythonBackend( QObject* parent,const QList<QVariant> args ) : Cantor::Backend( parent,args )
{
    kDebug()<<"Creating PythonBackend";

    setObjectName("pythonbackend");
}

PythonBackend::~PythonBackend()
{
    kDebug()<<"Destroying PythonBackend";
}

QString PythonBackend::id() const
{
    return "python";
}

Cantor::Session* PythonBackend::createSession()
{
    kDebug()<<"Spawning a new Python session";

    return new PythonSession(this);
}

Cantor::Backend::Capabilities PythonBackend::capabilities() const
{
    kDebug()<<"Requesting capabilities of PythonSession";

    return Cantor::Backend::Nothing;
}

bool PythonBackend::requirementsFullfilled() const
{
    QFileInfo info(PythonSettings::self()->path().toLocalFile());
    return info.isExecutable();
}

QWidget* PythonBackend::settingsWidget(QWidget* parent) const
{
    QWidget* widget=new QWidget(parent);
    Ui::PythonSettingsBase s;
    s.setupUi(widget);
    return widget;
}

KConfigSkeleton* PythonBackend::config() const
{
    return PythonSettings::self();
}

KUrl PythonBackend::helpUrl() const
{
    return i18nc("the url to the documentation of Scipy and Numpy packages", "http://docs.scipy.org/doc/");
}

QString PythonBackend::description() const
{
    return i18n("<p>Python is a remarkably powerful dynamic programming language that is used in a wide variety of application domains." \
                "There are several Python packages to scientific programming.</p>" \
                "<p>This backend supports <b>Scipy</b>, <b>Numpy</b>, and <b>Matplotlib</b> packages.</p>");
}

K_EXPORT_CANTOR_PLUGIN(pythonbackend, PythonBackend)

#include "pythonbackend.moc"
