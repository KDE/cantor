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
#include "maximahighlighter.h"

#include "kdebug.h"
#include <QWidget>

#include "mathematik_macros.h"


MaximaBackend::MaximaBackend( QObject* parent,const QList<QVariant> args ) : MathematiK::Backend( parent,args )
{
    setObjectName("maximabackend");
    kDebug()<<"Creating MaximaBackend";
    //initialize the supported extensions
    new MaximaHistoryExtension(this);
    new MaximaScriptExtension(this);
    new MaximaCASExtension(this);
    new MaximaCalculusExtension(this);
}

MaximaBackend::~MaximaBackend()
{
    kDebug()<<"Destroying MaximaBackend";
}

MathematiK::Session* MaximaBackend::createSession()
{
    kDebug()<<"Spawning a new Maxima session";

    return new MaximaSession(this);
}

MathematiK::Backend::Capabilities MaximaBackend::capabilities()
{
    kDebug()<<"Requesting capabilities of MaximaSession";
    return MathematiK::Backend::LaTexOutput|MathematiK::Backend::InteractiveMode|MathematiK::Backend::SyntaxHighlighting;
}

bool MaximaBackend::requirementsFullfilled()
{
    QFileInfo info(MaximaSettings::self()->path().toLocalFile());
    return info.isExecutable();
}

QSyntaxHighlighter* MaximaBackend::syntaxHighlighter(QTextEdit* parent)
{
    return new MaximaHighlighter(parent);
}


QWidget* MaximaBackend::settingsWidget(QWidget* parent)
{
    QWidget* widget=new QWidget(parent);
    Ui::MaximaSettingsBase s;
    s.setupUi(widget);
    return widget;
}

KConfigSkeleton* MaximaBackend::config()
{
    return MaximaSettings::self();
}

QString MaximaBackend::description()
{
    return i18n("Maxima is a system for the manipulation of symbolic and numerical expressions, "\
                "including differentiation, integration, Taylor series, Laplace transforms, "\
                "ordinary differential equations, systems of linear equations, polynomials, and sets, "\
                "lists, vectors, matrices, and tensors. Maxima yields high precision numeric results "\
                "by using exact fractions, arbitrary precision integers, and variable precision "\
                "floating point numbers. Maxima can plot functions and data in two and three dimensions. ");
}

K_EXPORT_MATHEMATIK_PLUGIN(maximabackend, MaximaBackend)

#include "maximabackend.moc"
