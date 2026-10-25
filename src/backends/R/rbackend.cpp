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

#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QStandardPaths>
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
    QString path;
#ifdef Q_OS_WIN
    path = QStandardPaths::findExecutable(QLatin1String("cantor_rserver.exe"));
#elif defined(Q_OS_MACOS)
    path = QStandardPaths::findExecutable(QLatin1String("cantor_rserver"));
    if (path.isEmpty()) {
        QDir appDir(QCoreApplication::applicationDirPath());
        appDir.cdUp(); appDir.cdUp(); appDir.cdUp(); // MacOS -> Contents -> cantor.app -> bin
        QString bundlePath = appDir.filePath(QLatin1String("cantor_rserver.app/Contents/MacOS/cantor_rserver"));
        if (QFile::exists(bundlePath))
            path = bundlePath;
    }
#else
    path = QStandardPaths::findExecutable(QLatin1String("cantor_rserver"));
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
    //description of R’s help system taken from https://www.r-project.org/help.html
    //@translators: don’t translate R’s keywords here ("help", etc.) which are put inside <code></code>
    return i18n("<div style=\"line-height: 1.4;\">"
    "<h3 style=\"color: #2277dd; margin: 12px 0 6px 0;\"><b>R Help System</b></h3>"
    "<p style=\"margin: 4px 0;\">The <code>help()</code> function and <code>?</code> help operator in R provide access to the documentation pages for R functions, data sets, and other objects.</p>"
    "<h4 style=\"color: #2277dd; margin: 12px 0 6px 0;\"><b>Examples</b></h4>"
    "<pre style=\"background-color: #d0d0d0; color: #000000; padding: 8px; border-radius: 4px;\">"
    "help(lm)      # help for linear model function\n"
    "?lm           # same as above\n"
    "?\"?\"          # help for the ? operator\n"
    "example(lm)   # run examples for lm</pre>"
    "<h4 style=\"color: #2277dd; margin: 12px 0 6px 0;\"><b>Package Help</b></h4>"
    "<p style=\"margin: 4px 0;\">To access help for a function in a package that’s not currently loaded, specify the package name:</p>"
    "<pre style=\"background-color: #d0d0d0; color: #000000; padding: 8px; border-radius: 4px;\">"
    "help(rlm, package=\"MASS\")\n"
    "help(package=\"MASS\")       # package index</pre>"
    "</div>");
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
