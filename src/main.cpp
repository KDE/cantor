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
#include <config-cantor.h>
#include <QApplication>
#include <KAboutData>
#include <KCrash>
#include <Kdelibs4ConfigMigrator>
#include <KLocalizedString>
#include <KConfigGroup>
#include <KMessageBox>
#include <QCommandLineParser>
#include <QUrl>
#include <QFileInfo>
#include <QDir>

static const char description[] =
    I18N_NOOP("KDE Frontend to mathematical applications");

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

#ifdef PATH_TO_CANTOR_PLUGINS
    // Add our custom plugins path, where we install our plugins, if it isn't default path
    const QString& path = QString::fromLocal8Bit(PATH_TO_CANTOR_PLUGINS);
    if (!QCoreApplication::libraryPaths().contains(path))
        QCoreApplication::addLibraryPath(path);
#else
    qDebug() << "PATH_TO_CANTOR_PLUGINS variable is not set (probably a bug), so use the default library paths";
#endif

    KCrash::initialize();

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

    KLocalizedString::setApplicationDomain("cantor");
    app.setApplicationName(QLatin1String("cantor"));
    app.setOrganizationDomain(QLatin1String("kde.org"));
    app.setApplicationDisplayName(i18n("Cantor"));
    app.setWindowIcon(QIcon::fromTheme(QLatin1String("cantor")));

    KAboutData about(QLatin1String("cantor"),
                     QLatin1String("Cantor"),
                     QLatin1String(CANTOR_VERSION),
                     i18n(description),
                     KAboutLicense::GPL,
                     i18n("(C) 2016 Filipe Saraiva<br/>(C) 2009-2015 Alexander Rieder"),
                     QString(),
                     QLatin1String("https://cantor.kde.org/"));

    about.addAuthor( i18n("Filipe Saraiva"), i18n("Maintainer<br/>Qt5/KF5 port, Scilab and Python backends"), QLatin1String("filipe@kde.org"), QLatin1String("http://filipesaraiva.info/") );
    about.addAuthor( i18n("Nikita Sirgienko"), i18nc("@info:credit", "Developer"), QLatin1String("warquark@gmail.com>"));
    about.addAuthor( i18n("Alexander Semke"), i18nc("@info:credit", "Developer"), QLatin1String("alexander.semke@web.de"));
    about.addAuthor( i18n("Alexander Rieder"), i18n("Original author<br/>Maintainer (2009 - 2015)"), QLatin1String("alexanderrieder@gmail.com") );
    about.addAuthor( i18n("Aleix Pol Gonzalez"), i18n("KAlgebra backend"), QLatin1String("aleixpol@kde.org") );
    about.addAuthor( i18n("Miha Čančula"), i18n("Octave backend"), QLatin1String("miha.cancula@gmail.com") );
    about.addAuthor( i18n("Martin Küttler"), i18n("Interface"), QLatin1String("martin.kuettler@gmail.com") );

    about.addCredit(QLatin1String("Andreas Kainz"), i18n("Cantor icon"), QLatin1String("kainz.a@gmail.com"));
    about.addCredit(QLatin1String("Uri Herrera"), i18n("Cantor icon"), QLatin1String("kaisergreymon99@gmail.com"), QLatin1String("http://nitrux.in/"));

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

        CantorShell *widget = new CantorShell();
        if ( parser.positionalArguments().count() == 0 )
        {
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
            const QStringList& args=parser.positionalArguments();
            for (const QString& filename : args)
            {
                const QUrl url = QUrl::fromUserInput(filename, QDir::currentPath(), QUrl::AssumeLocalFile);
                if (url.isValid())
                {
                    if (url.isLocalFile() && !QFileInfo(url.toLocalFile()).exists())
                        KMessageBox::error(widget, i18n("Couldn't open the file %1", filename), i18n("Cantor"));
                    else
                    {
                        widget->load(url);
                    }
                }
            }
        }
        widget->show();
    }

    return app.exec();
}

