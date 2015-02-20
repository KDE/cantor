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

#include "cantor.h"
#include <QApplication>
#include <KAboutData>
#include <Kdelibs4ConfigMigrator>
#include <KLocale>
#include <KConfigGroup>
#include <QCommandLineParser>

static const char description[] =
    I18N_NOOP("KDE Frontend to mathematical applications");

static const char version[] = "0.5";


int main(int argc, char **argv)
{
    // Migrating configuration from 4.x applications to KF5-based applications
    QStringList configFiles;
    QStringList rcFiles;

    configFiles << QLatin1String("cantorrc");
    rcFiles << QLatin1String("cantor_part.rc") << QLatin1String("cantor_scripteditor.rc")
            << QLatin1String("cantor_shell.rc") << QLatin1String("cantor_advancedplot_assistant.rc")
            << QLatin1String("cantor_differentiate_assistant.rc") << QLatin1String("cantor_import_package_assistant.rc")
            << QLatin1String("cantor_integrate_assistant.rc") << QLatin1String("cantor_create_matrix_assistant.rc")
            << QLatin1String("cantor_eigenvalues_assistant.rc") << QLatin1String("cantor_eigenvectors_assistant.rc")
            << QLatin1String("cantor_invert_matrix_assistant.rc") << QLatin1String("cantor_plot2d_assistant.rc")
            << QLatin1String("cantor_plot3d_assistant.rc") << QLatin1String("cantor_runscript_assistant.rc")
            << QLatin1String("cantor_solve_assistant.rc") << QLatin1String("cantor_qalculateplotassistant.rc");

    Kdelibs4ConfigMigrator migrator(QLatin1String("cantor"));

    migrator.setConfigFiles(configFiles);
    migrator.setUiFiles(rcFiles);
    migrator.migrate();
    //**********************************

    QApplication app(argc, argv);

    app.setApplicationName(QLatin1String("cantor"));
    app.setOrganizationDomain(QLatin1String("kde.org"));
    app.setApplicationDisplayName(i18n("Cantor"));
    app.setWindowIcon(QIcon::fromTheme(QLatin1String("cantor")));

    KAboutData about(QLatin1String("cantor"),
                     QLatin1String("Cantor"),
                     QLatin1String(version),
                     i18n(description),
                     KAboutLicense::GPL,
                     i18n("(C) 2009-2013 Alexander Rieder"),
                     QString(),
                     QLatin1String("http://edu.kde.org/cantor"));

    about.addAuthor( i18n("Alexander Rieder"), QString(), QLatin1String("alexanderrieder@gmail.com") );
    about.addAuthor( i18n("Aleix Pol Gonzalez"), i18n("KAlgebra backend"), QLatin1String("aleixpol@kde.org") );
    about.addAuthor( i18n("Miha Čančula"), i18n("Octave backend"), QLatin1String("miha.cancula@gmail.com") );
    about.addAuthor( i18n("Filipe Saraiva"), i18n("Scilab and Python backends"), QLatin1String("filipe@kde.org"),
                     QLatin1String("http://filipesaraiva.info/") );
    about.addAuthor( i18n("Martin Küttler"), i18n("Interface"), QLatin1String("martin.kuettler@gmail.com") );

    about.addCredit(QLatin1String("Andreas Kainz"), i18n("Cantor icon"), QLatin1String("kainz.a@gmail.com"));
    about.addCredit(QLatin1String("Uri Herrera"), i18n("Cantor icon"), QLatin1String("kaisergreymon99@gmail.com"),
                     QLatin1String("http://nitrux.in/"));

    QCommandLineParser parser;
    KAboutData::setApplicationData(about);
    parser.addVersionOption();
    parser.addHelpOption();

    const QCommandLineOption backendOption(QStringList()<<QLatin1String("b")<<QLatin1String("backend"), i18n("Use  backend <backend>"), QLatin1String("backend"));
    parser.addOption(backendOption);

    parser.addPositionalArgument(QStringLiteral("files"),  i18n("Documents to open."),  QStringLiteral("[files...]"));


    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);

    // see if we are starting with session management
    if (app.isSessionRestored())
        RESTORE(CantorShell)
    else
    {
        // no session.. just start up normally

        if ( parser.positionalArguments().count() == 0 )
        {
            CantorShell *widget = new CantorShell;
            widget->show();
            if(parser.isSet(QLatin1String("backend")))
            {
                widget->addWorksheet(parser.value(QLatin1String("backend")));
            }
            else
            {
                widget->addWorksheet();
            }
        }
        else
        {
            int i = 0;
            const QStringList& args=parser.positionalArguments();
            for (; i < args.count(); i++ )
            {
                CantorShell *widget = new CantorShell;
                widget->show();
                widget->load( QUrl::fromUserInput(args[i]) );
            }
        }

    }

    return app.exec();
}

