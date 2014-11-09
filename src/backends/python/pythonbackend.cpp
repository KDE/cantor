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
#include "pythonextensions.h"
#include "settings.h"
#include "ui_settings.h"

#include <QDebug>
#include <QWidget>

#include "cantor_macros.h"

#include <QLibrary>

PythonBackend::PythonBackend(QObject* parent,const QList<QVariant> args ) : Cantor::Backend( parent,args)
{
    qDebug()<<"Creating Python2Backend";

    new PythonLinearAlgebraExtension(this);
    new PythonPackagingExtension(this);
    new PythonPlotExtension(this);
    new PythonScriptExtension(this);
    new PythonVariableManagementExtension(this);

    setObjectName(QLatin1String("python2backend"));

    // Because the plugin may not have been loaded with
    // ExportExternalSymbols, we load the python symbols again
    // to make sure that python modules such as numpy see them
    // (see bug #330032)
    QLibrary pythonLib(QLatin1String("python2.7"));
    pythonLib.setLoadHints(QLibrary::ExportExternalSymbolsHint);
    pythonLib.load();
}

PythonBackend::~PythonBackend()
{
    qDebug()<<"Destroying Python2Backend";
}

QString PythonBackend::id() const
{
    return QLatin1String("python2");
}

Cantor::Session* PythonBackend::createSession()
{
    qDebug()<<"Spawning a new Python 2 session";

    return new PythonSession(this);
}

Cantor::Backend::Capabilities PythonBackend::capabilities() const
{
    qDebug()<<"Requesting capabilities of Python2Session";

    return Cantor::Backend::SyntaxHighlighting |
           Cantor::Backend::Completion         |
           Cantor::Backend::SyntaxHelp         |
           Cantor::Backend::VariableManagement;
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
    return i18nc("the url to the documentation Python 2", "http://docs.python.org/2/");
}

QString PythonBackend::description() const
{
    return i18n("<p>Python is a remarkably powerful dynamic programming language that is used in a wide variety of application domains. " \
                "There are several Python packages to scientific programming.</p>" \
                "<p>This backend supports Python 2.</p>");
}

K_EXPORT_CANTOR_PLUGIN(python2backend, PythonBackend)

#include "pythonbackend.moc"
