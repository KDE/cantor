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
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kconfiggroup.h>

static const char description[] =
    I18N_NOOP("KDE Frontend to mathematical applications");

static const char version[] = "0.5";

int main(int argc, char **argv)
{
    KAboutData about("cantor", 0,
                     ki18n("Cantor"),
                     version, ki18n(description),
                     KAboutData::License_GPL,
                     ki18n("(C) 2009-2013 Alexander Rieder"),
                     KLocalizedString(), 0
        );
    about.addAuthor( ki18n("Alexander Rieder"), KLocalizedString(), "alexanderrieder@gmail.com" );
    about.addAuthor( ki18n("Aleix Pol Gonzalez"), ki18n("KAlgebra backend"), "aleixpol@kde.org" );
    about.addAuthor( ki18n("Miha Čančula"), ki18n("Octave backend"), "miha.cancula@gmail.com" );
    about.addAuthor( ki18n("Filipe Saraiva"), ki18n("Scilab and Python backends"), "filipe@kde.org", "http://filipesaraiva.info/" );
    about.addAuthor( ki18n("Martin Küttler"), ki18n("Interface"), "martin.kuettler@gmail.com" );
    KCmdLineArgs::init(argc, argv, &about);

    KCmdLineOptions options;
    options.add("+[URL]", ki18n( "Document to open" ));
    options.add("backend [backend]", ki18n( "Use this backend" ));
    KCmdLineArgs::addCmdLineOptions( options );
    KApplication app;

    // see if we are starting with session management
    if (app.isSessionRestored())
        RESTORE(CantorShell)
    else
    {
        // no session.. just start up normally
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

        if ( args->count() == 0 )
        {
            CantorShell *widget = new CantorShell;
            widget->show();
            if(args->isSet("backend"))
            {
                widget->addWorksheet(args->getOption("backend"));
            }
            else
            {
                widget->addWorksheet();
            }
        }
        else
        {
            int i = 0;
            for (; i < args->count(); i++ )
            {
                CantorShell *widget = new CantorShell;
                widget->show();
                widget->load( args->url( i ) );
            }
        }
        args->clear();
    }

    return app.exec();
}
