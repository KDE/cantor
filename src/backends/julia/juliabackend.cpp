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

#include <QProcess>
#include <klocalizedstring.h>

#include <julia_version.h>

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
    return QLatin1String("1.0.0");
}

Cantor::Session *JuliaBackend::createSession()
{
    return new JuliaSession(this);
}

Cantor::Backend::Capabilities JuliaBackend::capabilities() const
{
    Cantor::Backend::Capabilities cap=
        SyntaxHighlighting|
        Completion;

    if (JuliaSettings::variableManagement())
        cap |= VariableManagement;

    return cap;
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

bool JuliaBackend::requirementsFullfilled(QString* const reason) const
{
    const QString& replPath = JuliaSettings::self()->replPath().toLocalFile();
    QFileInfo info(replPath);
    if (!info.isExecutable())
    {
        if (reason)
            *reason = i18n("You should set path to Julia executable");
        return false;
    }

    if (info.isSymLink())
    {
        if (reason)
            *reason = i18n("Path to Julia should point directly to julia executable, symlink not allowed");
        return false;
    }

    // Julia because of C API can handle only MAJOR.MINOR.* versions corresponding to
    // version, which used to build cantor_juliaserver
    // So check it and print info about it to user, if versions don't match
    QProcess getJuliaVersionProcess;
    getJuliaVersionProcess.setProgram(replPath);
    getJuliaVersionProcess.setArguments(QStringList()<<QLatin1String("-v"));
    getJuliaVersionProcess.start();
    getJuliaVersionProcess.waitForFinished(1000);

    QRegularExpression versionExp(QLatin1String("julia version (\\d+)\\.(\\d+).(\\d+)"));
    QString versionString = QString::fromLocal8Bit(getJuliaVersionProcess.readLine());
    QRegularExpressionMatch match = versionExp.match(versionString);
    if (getJuliaVersionProcess.state() != QProcess::NotRunning || !match.hasMatch())
    {
        if (reason)
            *reason = i18n("Сantor couldn’t determine the version of Julia for %1. Please specify the correct path to Julia executable (no symlinks allowed) and try again.", replPath);
        return false;
    }

    int juliaMajor = match.captured(1).toInt();
    int juliaMinor = match.captured(2).toInt();
    int juliaPatch = match.captured(3).toInt();

    if (QT_VERSION_CHECK(juliaMajor, juliaMinor, juliaPatch) != QT_VERSION_CHECK(JULIA_VERSION_MAJOR, JULIA_VERSION_MINOR, JULIA_VERSION_PATCH))
    {
        if (reason)
            *reason = i18n("You are trying to use Cantor with Julia v%1.%2.%3. This version of Cantor was compiled with the support of Julia v%4.%5.%6. Please point to this version of Julia or recompile Cantor using the version %1.%2.%3.", juliaMajor, juliaMinor, juliaPatch, JULIA_VERSION_MAJOR, JULIA_VERSION_MINOR, JULIA_VERSION_PATCH);
        return false;
    }
    return true;
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
