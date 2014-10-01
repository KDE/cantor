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

#include <KLocale>
#include <KConfigGroup>
#include <QCommandLineParser>

static const char description[] =
    I18N_NOOP("KDE Frontend to mathematical applications");

static const char version[] = "0.5";


int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    app.setApplicationName(QLatin1String("cantor"));
    app.setOrganizationDomain(QLatin1String("kde.org"));
    app.setApplicationDisplayName(i18n("Cantor"));

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


    QCommandLineParser parser;
    KAboutData::setApplicationData(about);
    parser.addVersionOption();
    parser.addHelpOption();

    parser.addOption(QCommandLineOption(QLatin1String("backend"),i18n("Use this backend")));;
    parser.addPositionalArgument(QLatin1String("file"), QCoreApplication::translate("main", "The file to open."));


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
                widget->load( QUrl(args[i]) );
            }
        }
        
    }

    return app.exec();
}

