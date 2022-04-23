/*
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2011 Matteo Agostinelli <agostinelli@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qalculatebackend.h"
#include "settings.h" // settings.h must be included before qalculatesession.h
#include "qalculatesession.h"
#include "qalculateextensions.h"
#include "qalculatesettingswidget.h"

#include <KLocalizedString>
#include <KPluginFactory>

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
    Q_UNUSED(reason);
    return true;
}

KConfigSkeleton* QalculateBackend::config() const
{
    return QalculateSettings::self();
}

QWidget* QalculateBackend::settingsWidget(QWidget* parent) const
{
    return new QalculateSettingsWidget(parent, id());
}

K_PLUGIN_FACTORY_WITH_JSON(qalculatebackend, "qalculatebackend.json", registerPlugin<QalculateBackend>();)
#include "qalculatebackend.moc"
