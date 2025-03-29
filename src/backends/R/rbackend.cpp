/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2019 Alexander Semke <alexander.semke@web.de>
*/

#include "rbackend.h"
#include "rsession.h"
#include "rextensions.h"
#include "settings.h"
#include "rsettingswidget.h"

#include <KPluginFactory>

RBackend::RBackend(QObject* parent,const QList<QVariant>& args) : Cantor::Backend(parent, args)
{
    new RScriptExtension(this);
    new RPlotExtension(this);
    new RVariableManagementExtension(this);
}

RBackend::~RBackend()
{
    qDebug()<<"Destroying RBackend";
}

QString RBackend::id() const
{
    return QLatin1String("r");
}

QString RBackend::version() const
{
    return QLatin1String("Undefined");
}

Cantor::Session* RBackend::createSession()
{
    qDebug()<<"Spawning a new R session";

    return new RSession(this);
}

Cantor::Backend::Capabilities RBackend::capabilities() const
{
    static Cantor::Backend::Capabilities cap=
        SyntaxHighlighting|
        Completion |
        InteractiveMode;

    if (RServerSettings::variableManagement())
        return cap |= VariableManagement;
    else
        return cap;
}

bool RBackend::requirementsFullfilled(QString* const reason) const
{
#ifdef Q_OS_WIN
    const QString& path = QStandardPaths::findExecutable(QLatin1String("cantor_rserver.exe"));
#else
    const QString& path = QStandardPaths::findExecutable(QLatin1String("cantor_rserver"));
#endif
    return Cantor::Backend::checkExecutable(QLatin1String("Cantor RServer"), path, reason);
}

QWidget* RBackend::settingsWidget(QWidget* parent) const
{
    return new RSettingsWidget(parent, id());
}

KConfigSkeleton* RBackend::config() const
{
    return RServerSettings::self();
}

QUrl RBackend::helpUrl() const
{
    return QUrl(i18nc("the url to the documentation of R, please check if there is a translated version and use the correct url",
                 "https://cran.r-project.org/manuals.html"));
}

QString RBackend::defaultHelp() const
{
    //description of R's help system taken from https://www.r-project.org/help.html
    //@tranlators: don't tranlate R's keywords here ("help", etc.) which are put inside <i></i>
    return i18n("<h1>R' Help System: <i>help()</i> and <i>?</i>:</h1><br>"
    "The <i>help()</i> function and <i>?</i> help operator in R provide access to the documentation pages for R functions, data sets, and other objects, both for packages in the standard R distribution and for contributed packages.<br><br>"
    "To access documentation for the standard <i>lm</i> (linear model) function, for example, enter the command <b><i>help(lm)</i></b> or <i>help(\"lm\")</i>, or <i>?lm</i> or <i>?\"lm\"</i> (i.e., the quotes are optional).<br><br>"
    "To access help for a function in a package that’s not currently loaded, specify in addition the name of the package: For example, to obtain documentation for the <i>rlm()</i> (robust linear model) function in the MASS package, <i>help(rlm, package=\"MASS\")</i>.<br><br>"
    "Standard names in R consist of upper- and lower-case letters, numerals (0-9), underscores (_), and periods (.), and must begin with a letter or a period. To obtain help for an object with a non-standard name (such as the help operator <i>?</i>), the name must be quoted: for example, <i>help('?')</i> or <i>?\"?\"</i>.<br><br>"
    "You may also use the <i>help()</i> function to access information about a package in your library — for example, <i>help(package=\"MASS\")</i> — which displays an index of available help pages for the package along with some other information.<br><br>"
    "Help pages for functions usually include a section with executable examples illustrating how the functions work. You can execute these examples in the current R session via the <i>example()</i> command: e.g., <i>example(lm)</i>.");
}

QString RBackend::description() const
{
    return i18n("<b>R</b> is a language and environment for statistical computing and graphics, similar to the S language and environment. <br/>"\
                "It provides a wide variety of statistical (linear and nonlinear modelling, "\
                "classical statistical tests, time-series analysis, classification, clustering, ...) "\
                "and graphical techniques, and is highly extensible. The S language is often the "\
                "vehicle of choice for research in statistical methodology, "\
                "and R provides an Open Source route to participation in that activity.");
}

K_PLUGIN_FACTORY_WITH_JSON(rbackend, "rbackend.json", registerPlugin<RBackend>();)
#include "rbackend.moc"
