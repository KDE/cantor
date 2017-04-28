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
    Copyright (C) 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
 */
#include "juliabackend.h"

#include <klocalizedstring.h>

#include "juliasession.h"
#include "ui_settings.h"
#include "settings.h"
#include "juliaextensions.h"

JuliaBackend::JuliaBackend(QObject *parent, const QList<QVariant> &args)
    : Cantor::Backend(parent, args)
{
    setEnabled(true);

    new JuliaVariableManagementExtension(this);
    new JuliaPackagingExtension(this);
    new JuliaPlotExtension(this);
    new JuliaScriptExtension(this);
    new JuliaLinearAlgebraExtension(this);
}

QString JuliaBackend::id() const
{
    return QLatin1String("julia");
}

QString JuliaBackend::version() const
{
    return QLatin1String("0.4");
}

Cantor::Session *JuliaBackend::createSession()
{
    return new JuliaSession(this);
}

Cantor::Backend::Capabilities JuliaBackend::capabilities() const
{
    return Cantor::Backend::SyntaxHighlighting |
        Cantor::Backend::VariableManagement |
        Cantor::Backend::Completion;
}

QString JuliaBackend::description() const
{
    return i18n(
        "<p><b>Julia</b> is a high-level, high-performance dynamic programming "
        "language for technical computing, with syntax that is familiar to "
        "users of other technical computing environments. It provides a "
        "sophisticated compiler, distributed parallel execution, numerical "
        "accuracy, and an extensive mathematical function library.</p>"
    );
}

QUrl JuliaBackend::helpUrl() const
{
    return QUrl(i18nc(
        "The url to the documentation of Julia, please check if there is a"
        " translated version and use the correct url",
        "http://docs.julialang.org/en/latest/"
    ));
}

bool JuliaBackend::requirementsFullfilled() const
{
    return QFileInfo(
        JuliaSettings::self()->replPath().toLocalFile()
    ).isExecutable();
}

QWidget *JuliaBackend::settingsWidget(QWidget *parent) const
{
    QWidget *widget = new QWidget(parent);
    Ui::JuliaSettingsBase s;
    s.setupUi(widget);
    return widget;
}

KConfigSkeleton *JuliaBackend::config() const
{
    return JuliaSettings::self();
}

K_PLUGIN_FACTORY_WITH_JSON(
    juliabackend,
    "juliabackend.json",
    registerPlugin<JuliaBackend>();
)

#include "juliabackend.moc"
