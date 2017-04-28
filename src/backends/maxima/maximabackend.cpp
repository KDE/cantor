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
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
 */

#include "maximabackend.h"

#include "maximasession.h"
#include "settings.h"
#include "ui_settings.h"
#include "maximaextensions.h"

#include <QDebug>
#include <QWidget>

#include "cantor_macros.h"


MaximaBackend::MaximaBackend( QObject* parent,const QList<QVariant> args ) : Cantor::Backend( parent,args )
{
    setObjectName(QLatin1String("maximabackend"));
    qDebug()<<"Creating MaximaBackend";
    //initialize the supported extensions
    new MaximaHistoryExtension(this);
    new MaximaScriptExtension(this);
    new MaximaCASExtension(this);
    new MaximaCalculusExtension(this);
    new MaximaLinearAlgebraExtension(this);
    new MaximaPlotExtension(this);
    new MaximaVariableManagementExtension(this);
}

MaximaBackend::~MaximaBackend()
{
    qDebug()<<"Destroying MaximaBackend";
}

QString MaximaBackend::id() const
{
    return QLatin1String("maxima");
}

QString MaximaBackend::version() const
{
    return QLatin1String("5.38 and 5.39");
}

Cantor::Session* MaximaBackend::createSession()
{
    qDebug()<<"Spawning a new Maxima session";

    return new MaximaSession(this);
}

Cantor::Backend::Capabilities MaximaBackend::capabilities() const
{
    qDebug()<<"Requesting capabilities of MaximaSession";
    Cantor::Backend::Capabilities cap=
        Cantor::Backend::LaTexOutput |
        Cantor::Backend::InteractiveMode|
        Cantor::Backend::SyntaxHighlighting|
        Cantor::Backend::Completion |
        Cantor::Backend::SyntaxHelp;
    if(MaximaSettings::self()->variableManagement())
        cap|=Cantor::Backend::VariableManagement;

    return cap;
}

bool MaximaBackend::requirementsFullfilled() const
{
    QFileInfo info(MaximaSettings::self()->path().toLocalFile());
    return info.isExecutable();
}

QUrl MaximaBackend::helpUrl() const
{
    return QUrl(i18nc("the url to the documentation of Maxima, please check if there is a translated version and use the correct url",
                 "http://maxima.sourceforge.net/docs/manual/en/maxima.html"));
}

QWidget* MaximaBackend::settingsWidget(QWidget* parent) const
{
    QWidget* widget=new QWidget(parent);
    Ui::MaximaSettingsBase s;
    s.setupUi(widget);
    return widget;
}

KConfigSkeleton* MaximaBackend::config() const
{
    return MaximaSettings::self();
}

QString MaximaBackend::description() const
{
    return i18n("Maxima is a system for the manipulation of symbolic and numerical expressions, "\
                "including differentiation, integration, Taylor series, Laplace transforms, "\
                "ordinary differential equations, systems of linear equations, polynomials, and sets, "\
                "lists, vectors, matrices, and tensors. Maxima yields high precision numeric results "\
                "by using exact fractions, arbitrary precision integers, and variable precision "\
                "floating point numbers. Maxima can plot functions and data in two and three dimensions. ");
}

K_PLUGIN_FACTORY_WITH_JSON(maximabackend, "maximabackend.json", registerPlugin<MaximaBackend>();)
#include "maximabackend.moc"
