/************************************************************************************
*  Copyright (C) 2009 by Milian Wolff <mail@milianw.de>                             *
*  Copyright (C) 2011 by Matteo Agostinelli <agostinelli@gmail.com>                 *
*                                                                                   *
*  This program is free software; you can redistribute it and/or                    *
*  modify it under the terms of the GNU General Public License                      *
*  as published by the Free Software Foundation; either version 2                   *
*  of the License, or (at your option) any later version.                           *
*                                                                                   *
*  This program is distributed in the hope that it will be useful,                  *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
*  GNU General Public License for more details.                                     *
*                                                                                   *
*  You should have received a copy of the GNU General Public License                *
*  along with this program; if not, write to the Free Software                      *
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
*************************************************************************************/

#include "qalculatebackend.h"

// settings.h must be included before qalculatesession.h
#include "ui_settings.h"
#include "settings.h"

#include "qalculatesession.h"
#include "qalculateextensions.h"

#include <KLocalizedString>

QalculateBackend::QalculateBackend(QObject* parent,const QList<QVariant> args)
  : Cantor::Backend(parent, args)
{
    new QalculateHistoryExtension(this);
    new QalculateVariableManagementExtension(this);
    new QalculateCalculusExtension(this);
    new QalculateCASExtension(this);
    new QalculateLinearAlgebraExtension(this);
    new QalculatePlotExtension(this);
}

QString QalculateBackend::id() const
{
    return QLatin1String("qalculate");
}

QString QalculateBackend::version() const
{
    return QLatin1String("0.9");
}

Cantor::Session* QalculateBackend::createSession()
{
    return new QalculateSession(this);
}

Cantor::Backend::Capabilities QalculateBackend::capabilities() const
{
    return Cantor::Backend::Completion | Cantor::Backend::SyntaxHighlighting | Cantor::Backend::SyntaxHelp | Cantor::Backend::VariableManagement;
//     return Cantor::Backend::Completion | Cantor::Backend::SyntaxHelp;
}

QString QalculateBackend::description() const
{
    return i18n("<b>Qalculate!</b> is not your regular software replication of the cheapest available calculator. Qalculate! aims to make full use of the superior interface, power and flexibility of modern computers. "\
    "The center of attention in Qalculate! is the expression entry. Instead of entering each number in a mathematical expression separately, you can directly write the whole expression and later modify it. "\
    "The interpretation of expressions is flexible and fault tolerant, and if you nevertheless do something wrong, Qalculate! will tell you so. Not fully solvable expressions are however not errors. Qalculate! will simplify as far as it can and answer with an expression. "\
    "In addition to numbers and arithmetic operators, an expression may contain any combination of variables, units, and functions.");
}

QUrl QalculateBackend::helpUrl() const
{
    // A sub-optimal solution but still this manual is fairly complete
    return QUrl(QString::fromLatin1("https://qalculate.github.io/manual/index.html"));
}

bool QalculateBackend::requirementsFullfilled(QString* const reason) const
{
    return true;
}

KConfigSkeleton* QalculateBackend::config() const
{
    return QalculateSettings::self();
}

QWidget* QalculateBackend::settingsWidget(QWidget* parent) const
{
    QWidget* widget = new QWidget(parent);
    Ui::QalculateSettingsBase s;
    s.setupUi(widget);

    return widget;
}

K_PLUGIN_FACTORY_WITH_JSON(qalculatebackend, "qalculatebackend.json", registerPlugin<QalculateBackend>();)
#include "qalculatebackend.moc"
