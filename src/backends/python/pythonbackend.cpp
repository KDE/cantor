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
#include "ui_settings.h"

#include <QDebug>
#include <QWidget>

PythonBackend::PythonBackend(QObject* parent, const QList<QVariant> args) : Cantor::Backend(parent, args)
{
    qDebug()<<"Creating PythonBackend";

    new PythonLinearAlgebraExtension(this);
    new PythonPackagingExtension(this);
    new PythonPlotExtension(this);
    new PythonScriptExtension(this);
    new PythonVariableManagementExtension(this);
}

PythonBackend::~PythonBackend()
{
    qDebug()<<"Destroying PythonBackend";
}

Cantor::Backend::Capabilities PythonBackend::capabilities() const
{
    qDebug()<<"Requesting capabilities of PythonSession";

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

