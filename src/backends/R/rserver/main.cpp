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

#include "rserver.h"

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kconfiggroup.h>

static const char description[] =
    I18N_NOOP("Server for the MathematiK R Backend");

static const char version[] = "0.1";

int main(int argc, char **argv)
{
    KAboutData about("mathematik_rserver", 0,
                     ki18n("MathematiK Server for R"),
                     version, ki18n(description),
                     KAboutData::License_GPL,
                     ki18n("(C) 2009 Alexander Rieder"),
                     KLocalizedString(), 0,
                     "alexanderrieder@gmail.com"
        );
    about.addAuthor( ki18n("Alexander Rieder"), KLocalizedString(), "alexanderrieder@gmail.com" );
    KCmdLineArgs::init(argc, argv, &about);


    KApplication app;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    RServer * server=new RServer();

    return app.exec();
}

