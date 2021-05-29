/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2010 Miha Čančula <miha.cancula@gmail.com>
    SPDX-FileCopyrightText: 2019 Alexander Semke <alexander.semke@web.de>
*/

#include "octavebackend.h"
#include "octaveextensions.h"
#include "octavesession.h"
#include "octavesettingswidget.h"
#include "settings.h"

OctaveBackend::OctaveBackend(QObject* parent, const QList<QVariant>& args): Backend(parent, args)
{
    new OctaveHistoryExtension(this);
    new OctaveScriptExtension(this);
    new OctavePlotExtension(this);
    new OctaveLinearAlgebraExtension(this);
    new OctaveVariableManagementExtension(this);
    new OctavePackagingExtension(this);
}

QString OctaveBackend::id() const
{
    return QLatin1String("octave");
}

QString OctaveBackend::version() const
{
    return QLatin1String("5.2");
}

Cantor::Backend::Capabilities OctaveBackend::capabilities() const
{
    Cantor::Backend::Capabilities cap =
        SyntaxHighlighting |
        Completion         |
        SyntaxHelp         |
        IntegratedPlots;

    if (OctaveSettings::self()->variableManagement())
        cap |= VariableManagement;
    return cap;
}

Cantor::Session* OctaveBackend::createSession()
{
    return new OctaveSession(this);
}

bool OctaveBackend::requirementsFullfilled(QString* const reason) const
{
    const QString& path = OctaveSettings::path().toLocalFile();
    return Cantor::Backend::checkExecutable(QLatin1String("Octave"), path, reason);
}

QUrl OctaveBackend::helpUrl() const
{
    const QUrl& localDoc = OctaveSettings::self()->localDoc();
    if (!localDoc.isEmpty())
        return localDoc;
    else
        return QUrl(i18nc("the url to the documentation of Octave, please check if there is a translated version (currently Czech and Japanese) and use the correct url",
            "https://octave.org/doc/interpreter/"));
}

QString OctaveBackend::description() const
{
    return i18n("<b>GNU Octave</b> is a high-level language, primarily intended for numerical computations. <br/>"
                "It provides a convenient command line interface for solving linear and nonlinear problems numerically, and for performing other numerical experiments using a language that is mostly compatible with Matlab.");
}

QWidget* OctaveBackend::settingsWidget(QWidget* parent) const
{
    return new OctaveSettingsWidget(parent, id());
}

KConfigSkeleton* OctaveBackend::config() const
{
    return OctaveSettings::self();
}

K_PLUGIN_FACTORY_WITH_JSON(octavebackend, "octavebackend.json", registerPlugin<OctaveBackend>();)
#include "octavebackend.moc"
