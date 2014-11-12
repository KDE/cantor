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

#include <QApplication>
#include <KAboutData>
#include <QCommandLineParser>
#include <KLocale>
#include <KConfigGroup>

static const char description[] =
    I18N_NOOP("Server for the Cantor R Backend");

static const char version[] = "0.1";

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    app.setApplicationName(QLatin1String("R Server"));
    app.setOrganizationDomain(QLatin1String("kde.org"));
    app.setApplicationDisplayName(i18n("R Server"));

    KAboutData about(QLatin1String("cantor_rserver"),
                     QLatin1String("cantor_rserver"),
                     QLatin1String(version),
                     i18n(description),
                     KAboutLicense::GPL,
                     i18n("(C) 2009 Alexander Rieder"),
                     QString(),
                     QLatin1String("alexanderrieder@gmail.com"));

    about.addAuthor( i18n("Alexander Rieder"), QString(), QLatin1String("alexanderrieder@gmail.com") );
    KAboutData::setApplicationData(about);

    QCommandLineParser parser;

    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);

    new RServer();

    return app.exec();
}

