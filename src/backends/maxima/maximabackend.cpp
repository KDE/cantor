/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2019 Alexander Semke <alexander.semke@web.de>
*/

#include "maximabackend.h"
#include "maximaextensions.h"
#include "maximasession.h"
#include "maximasettingswidget.h"
#include "settings.h"

#include <KPluginFactory>

MaximaBackend::MaximaBackend( QObject* parent,const QList<QVariant> args ) : Cantor::Backend( parent,args )
{
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
    return QLatin1String("5.41, 5.42");
}

Cantor::Session* MaximaBackend::createSession()
{
    qDebug()<<"Spawning a new Maxima session";

    return new MaximaSession(this);
}

Cantor::Backend::Capabilities MaximaBackend::capabilities() const
{
    Cantor::Backend::Capabilities cap =
        Cantor::Backend::LaTexOutput |
        Cantor::Backend::InteractiveMode|
        Cantor::Backend::SyntaxHighlighting|
        Cantor::Backend::Completion |
        Cantor::Backend::SyntaxHelp;
    if(MaximaSettings::self()->variableManagement())
        cap |= Cantor::Backend::VariableManagement;

    return cap;
}

bool MaximaBackend::requirementsFullfilled(QString* const reason) const
{
    const QString& path = MaximaSettings::self()->path().toLocalFile();
    return Cantor::Backend::checkExecutable(QLatin1String("Maxima"), path, reason);
}

QUrl MaximaBackend::helpUrl() const
{
    return QUrl(i18nc("the url to the documentation of Maxima, please check if there is a translated version and use the correct url",
            "http://maxima.sourceforge.net/docs/manual/en/maxima.html"));
}

QString MaximaBackend::defaultHelp() const
{
    //taken from https://maxima.sourceforge.io/docs/manual/maxima_5.html
    //@tranlators: don't tranlate Maxima's keywords here ("describe", "true", "false", etc.) which are put inside of <i></i>
    return i18n("<h1>Maxima's Help System</h1>"
                "<h2>Function <i>apropos</i></h2>"
                "Searches for Maxima names which have name appearing anywhere within them; <i>name</i> must be a string or symbol. Thus, <i>apropos(exp)</i> returns a list of all the flags and functions which have exp as part of their name, such as <i>expand</i>, <i>exp</i>, and <i>exponentialize</i>. So, if you can only remember part of the name of a Maxima command or variable, you can use this command to find the rest of the name. Similarly, you can type <i>apropos(tr_)</i> to find a list of many of the switches relating to the translator, most of which begin with <i>tr_</i>.<br><br>"
                "<i>apropos(\"\")</i> returns a list with all Maxima names.<br><br>"
                "<i>apropos</i> returns the empty list [], if no name is found.<br><br>"
                "<h2>Function <i>describe</i></h2>"
                "<i>describe(string)</i> is equivalent to describe(string, exact).<br><br>"
                "<i>describe(string, exact)</i> finds an item with title equal (case-insensitive) to string, if there is any such item.<br><br>"
                "<i>describe(string, inexact)</i> finds all documented items which contain string in their titles. If there is more than one such item, Maxima asks the user to select an item or items to display.<br><br>"
                "<i>? foo</i> (with a space between <i>?</i> and <i>foo</i>) is equivalent to <i>describe(\"foo\", exact)</i>, and <i>?? foo</i> is equivalent to <i>describe(\"foo\", inexact)</i>.<br><br>"
                "<i>describe("", inexact)</i> yields a list of all topics documented in the on-line manual.<br><br>"
                "<i>describe</i> quotes its argument. <i>describe</i> returns <i>true</i> if some documentation is found, otherwise <i>false</i>.<br>"
                "<h2>Function <i>example</i></h2>"
                "<i>example(topic)</i> displays some examples of <i>topic</i>, which is a symbol or a string. To get examples for operators like <i>if</i>, <i>do</i>, or <i>lambda</i> the argument must be a string, e.g. <i>example(\"do\")</i>. <i>example</i> is not case sensitive. Most topics are function names.<br><br>"
                "<i>example()</i> returns the list of all recognized topics.<br><br>"
                "The name of the file containing the examples is given by the global option variable <i>manual_demo</i>, which defaults to <i>\"manual.demo\"</i>.<br><br>"
                "<i>example</i> quotes its argument. <i>example</i> returns <i>done</i> unless no examples are found or there is no argument, in which case <i>example</i> returns the list of all recognized topics."
                );
}

QWidget* MaximaBackend::settingsWidget(QWidget* parent) const
{
    return new MaximaSettingsWidget(parent, id());
}

KConfigSkeleton* MaximaBackend::config() const
{
    return MaximaSettings::self();
}

QString MaximaBackend::description() const
{
    return i18n("<b>Maxima</b> is a system for the manipulation of symbolic and numerical expressions, "\
                "including differentiation, integration, Taylor series, Laplace transforms, "\
                "ordinary differential equations, systems of linear equations, polynomials, and sets, "\
                "lists, vectors, matrices, and tensors. Maxima yields high precision numeric results "\
                "by using exact fractions, arbitrary precision integers, and variable precision "\
                "floating point numbers. Maxima can plot functions and data in two and three dimensions.");
}

K_PLUGIN_FACTORY_WITH_JSON(maximabackend, "maximabackend.json", registerPlugin<MaximaBackend>();)
#include "maximabackend.moc"
