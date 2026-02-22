/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2016-2026 Alexander Semke <alexander.semke@web.de>
*/

#include "cantor.h"
#include <config-cantor.h>

#include <QApplication>
#include <KAboutData>
#include <KCrash>
#include <KLocalizedString>
#include <KConfigGroup>
#include <KMessageBox>

#include <QCommandLineParser>
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

#ifdef HAVE_EMBEDDED_DOCUMENTATION
#include <QWebEngineUrlScheme>
#endif

int main(int argc, char **argv)
{
#ifdef HAVE_EMBEDDED_DOCUMENTATION
    // Register custom scheme handler for qthelp:// scheme
    QWebEngineUrlScheme qthelp("qthelp");
    QWebEngineUrlScheme::registerScheme(qthelp);
#endif

    QApplication app(argc, argv);

    // Add our custom plugins path, where we install our plugins, if it isn't default path
    const QString& path = QString::fromLocal8Bit(PATH_TO_CANTOR_PLUGINS);
    qDebug() << "Adding additional application library path for Cantor plugins loading" << path;
    if (!QCoreApplication::libraryPaths().contains(path))
        QCoreApplication::addLibraryPath(path);

    KCrash::initialize();

    KLocalizedString::setApplicationDomain("cantor");
    app.setApplicationName(QLatin1String("cantor"));
    app.setOrganizationDomain(QLatin1String("kde.org"));
    app.setApplicationDisplayName(i18n("Cantor"));
    app.setWindowIcon(QIcon::fromTheme(QLatin1String("cantor")));

    KAboutData about(QLatin1String("cantor"),
                     QLatin1String("Cantor"),
                     QLatin1String(CANTOR_VERSION),
                     i18n("KDE Frontend to mathematical applications"),
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

    const QCommandLineOption backendOption(QStringList()<<QLatin1String("b")<<QLatin1String("backend"), i18n("Use  backend <backend>"), QLatin1String("backend"));
    parser.addOption(backendOption);
    parser.addPositionalArgument(QStringLiteral("files"),  i18n("Documents to open."),  QStringLiteral("[files...]"));

    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);

    // see if we are starting with session management
    if (app.isSessionRestored())
        kRestoreMainWindows<CantorShell>();
    else
    {
        // no session.. just start up normally
        auto* widget = new CantorShell();
        if ( parser.positionalArguments().count() == 0 )
        {
            if(parser.isSet(QLatin1String("backend")))
                widget->addWorksheet(parser.value(QLatin1String("backend")));
            else
                widget->addWorksheet();
        }
        else
        {
            const auto& args = parser.positionalArguments();
            for (const QString& filename : args)
            {
                const QUrl url = QUrl::fromUserInput(filename, QDir::currentPath(), QUrl::AssumeLocalFile);
                if (url.isValid())
                {
                    if (url.isLocalFile() && !QFileInfo(url.toLocalFile()).exists())
                        KMessageBox::error(widget, i18n("Couldn't open the file %1", filename), i18n("Cantor"));
                    else
                        widget->load(url);
                }
            }
        }
        widget->show();
    }

    return app.exec();
}
