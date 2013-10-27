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

#include "python2backend.h"

#include "python2session.h"
#include "python2extensions.h"
#include "settings.h"
#include "ui_settings.h"

#include "kdebug.h"
#include <QWidget>

#include "cantor_macros.h"

Python2Backend::Python2Backend(QObject* parent,const QList<QVariant> args ) : Cantor::Backend( parent,args)
{
    kDebug()<<"Creating Python2Backend";

    new Python2PackagingExtension(this);
    new Python2VariableManagementExtension(this);

    setObjectName("python2backend");
}

Python2Backend::~Python2Backend()
{
    kDebug()<<"Destroying Python2Backend";
}

QString Python2Backend::id() const
{
    return "python2";
}

Cantor::Session* Python2Backend::createSession()
{
    kDebug()<<"Spawning a new Python 2 session";

    return new Python2Session(this);
}

Cantor::Backend::Capabilities Python2Backend::capabilities() const
{
    kDebug()<<"Requesting capabilities of Python2Session";

    return Cantor::Backend::SyntaxHighlighting |
           Cantor::Backend::Completion         |
           Cantor::Backend::SyntaxHelp         |
           Cantor::Backend::VariableManagement;
}

bool Python2Backend::requirementsFullfilled() const
{
    QFileInfo info(Python2Settings::self()->path().toLocalFile());
    return info.isExecutable();
}

QWidget* Python2Backend::settingsWidget(QWidget* parent) const
{
    QWidget* widget=new QWidget(parent);
    Ui::Python2SettingsBase s;
    s.setupUi(widget);
    return widget;
}

KConfigSkeleton* Python2Backend::config() const
{
    return Python2Settings::self();
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
