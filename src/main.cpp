#include "mathematik.h"
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kconfiggroup.h>

static const char description[] =
    I18N_NOOP("A KDE Frontend to mathematical applications");

static const char version[] = "0.1";

int main(int argc, char **argv)
{
    KAboutData about("mathematik", 0,
                     ki18n("MathematiK"),
                     version, ki18n(description),
                     KAboutData::License_GPL,
                     ki18n("(C) 2008 Alexander Rieder"),
                     KLocalizedString(), 0,
                     "alexanderrieder@gmail.com"
        );
    about.addAuthor( ki18n("Alexander Rieder"), KLocalizedString(), "alexanderrieder@gmail.com" );
    KCmdLineArgs::init(argc, argv, &about);

    KCmdLineOptions options;
    options.add("+[URL]", ki18n( "Document to open" ));
    options.add("backend [backend]", ki18n( "Use this backend" ));
    KCmdLineArgs::addCmdLineOptions( options );
    KApplication app;

    // see if we are starting with session management
    if (app.isSessionRestored())
        RESTORE(MathematiKShell)
    else
    {
        // no session.. just start up normally
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

        if ( args->count() == 0 )
        {
            MathematiKShell *widget = new MathematiKShell;
            widget->show();
        }
        else
        {
            int i = 0;
            for (; i < args->count(); i++ )
            {
                MathematiKShell *widget = new MathematiKShell;
                widget->show();
                widget->load( args->url( i ) );
            }
        }
        args->clear();
    }

    return app.exec();
}
