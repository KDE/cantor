/*
    SPDX-FileCopyrightText: 2010 Miha Čančula <miha.cancula@gmail.com>
    SPDX-FileCopyrightText: 2018-2022 by Alexander Semke (alexander.semke@web.de)

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "octaveexpression.h"
#include "octavesession.h"
#include "defaultvariablemodel.h"
#include <helpresult.h>
#include "imageresult.h"
#include "settings.h"
#include "textresult.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QFileSystemWatcher>
#include <QRegularExpression>

#include <KLocalizedString>

static const QString printCommandTemplate = QString::fromLatin1("print(\"%1\", \"-S%2,%3\")");

static const QStringList plotCommands({
    QLatin1String("plot"), QLatin1String("semilogx"), QLatin1String("semilogy"),
    QLatin1String("loglog"), QLatin1String("polar"), QLatin1String("contour"),
    QLatin1String("bar"), QLatin1String("stairs"), QLatin1String("errorbar"),
    QLatin1String("sombrero"), QLatin1String("hist"), QLatin1String("fplot"),
    QLatin1String("imshow"), QLatin1String("stem"), QLatin1String("stem3"),
    QLatin1String("scatter"), QLatin1String("pareto"), QLatin1String("rose"),
    QLatin1String("pie"), QLatin1String("quiver"), QLatin1String("compass"),
    QLatin1String("feather"), QLatin1String("pcolor"), QLatin1String("area"),
    QLatin1String("fill"), QLatin1String("comet"), QLatin1String("plotmatrix"),
    /* 3d-plots */
    QLatin1String("plot3"), QLatin1String("mesh"), QLatin1String("meshc"),
    QLatin1String("meshz"), QLatin1String("surf"), QLatin1String("surfc"),
    QLatin1String("surfl"), QLatin1String("surfnorm"), QLatin1String("isosurface"),
    QLatin1String("isonormals"), QLatin1String("isocaps"),
    /* 3d-plots defined by a function */
    QLatin1String("ezplot3"), QLatin1String("ezmesh"), QLatin1String("ezmeshc"),
    QLatin1String("ezsurf"), QLatin1String("ezsurfc"), QLatin1String("cantor_plot2d"),
    QLatin1String("cantor_plot3d")});

const QStringList OctaveExpression::plotExtensions({
    QLatin1String("pdf"),
    QLatin1String("svg"),
    QLatin1String("png")
});

OctaveExpression::OctaveExpression(Cantor::Session* session, bool internal): Expression(session, internal)
{
}

OctaveExpression::~OctaveExpression()
{
}

void OctaveExpression::evaluate()
{
    m_plotFilename.clear();

    m_finished = false;
    m_plotPending = false;

    session()->enqueueExpression(this);
}

QString OctaveExpression::internalCommand()
{
    QString cmd = command();
    auto* octaveSession = static_cast<OctaveSession*>(session());

    if (octaveSession->isIntegratedPlotsEnabled() && !isInternal())
    {
        QStringList cmdWords = cmd.split(QRegularExpression(QStringLiteral("\\b")), QString::SkipEmptyParts);
        if (!cmdWords.contains(QLatin1String("help")) && !cmdWords.contains(QLatin1String("completion_matches")))
        {
            for (const QString& plotCmd : plotCommands)
                if (cmdWords.contains(plotCmd))
                {

                    if (!cmd.endsWith(QLatin1Char(';')) && !cmd.endsWith(QLatin1Char(',')))
                        cmd += QLatin1Char(',');

                    m_plotFilename = octaveSession->plotFilePrefixPath() + QString::number(id()) + QLatin1String(".") + plotExtensions[OctaveSettings::inlinePlotFormat()];

                    int w, h;
                    if (OctaveSettings::inlinePlotFormat() == 0 || OctaveSettings::inlinePlotFormat() == 1) // for vector formats like PDF and SVG the size for 'print'  is provided in points
                    {
                        w = OctaveSettings::plotWidth() / 2.54 * 72;
                        h = OctaveSettings::plotHeight() / 2.54 * 72;
                    }
                    else // for raster formats the size for 'print' is provided in pixels
                    {
                        w = OctaveSettings::plotWidth() / 2.54 * QApplication::desktop()->physicalDpiX();
                        h = OctaveSettings::plotHeight() / 2.54 * QApplication::desktop()->physicalDpiX();
                    }
                    cmd += printCommandTemplate.arg(m_plotFilename, QString::number(w), QString::number(h));

                    auto* watcher = fileWatcher();
                    if (!watcher->files().isEmpty())
                        watcher->removePaths(watcher->files());

                    // add path works only for existing paths, so create the file
                    QFile file(m_plotFilename);
                    if (file.open(QFile::WriteOnly))
                    {
                        file.close();
                        watcher->addPath(m_plotFilename);
                        m_plotPending = true;
                        connect(watcher, &QFileSystemWatcher::fileChanged, this, &OctaveExpression::imageChanged,  Qt::UniqueConnection);
                    }
                    break;
                }
        }
    }

    // We need to remove all comments here, because below we merge all strings to one long string
    // Otherwise, all code after line with comment will be commented out after merging
    // So, this small state machine remove all comments
    // FIXME better implementation
    QString tmp;
    // 0 - command mode, 1 - string mode for ', 2 - string mode for ", 3 - comment mode
    int status = 0;
    for (int i = 0; i < cmd.size(); i++)
    {
        const char ch = cmd[i].toLatin1();
        if (status == 0 && (ch == '#' || ch == '%'))
            status = 3;
        else if (status == 0 && ch == '\'')
            status = 1;
        else if (status == 0 && ch == '"')
            status = 2;
        else if (status == 1 && ch == '\'')
            status = 0;
        else if (status == 2 && ch == '"')
            status = 0;
        else if (status == 3 && ch == '\n')
            status = 0;

        if (status != 3)
            tmp += cmd[i];
    }

    //Remove "\n" in the beginning of the command, if present
    while(tmp[0] == QLatin1Char('\n'))
        tmp.remove(0, 1);

    cmd = tmp;
    cmd.replace(QLatin1String(";\n"), QLatin1String(";"));
    cmd.replace(QLatin1Char('\n'), QLatin1Char(','));
    cmd += QLatin1Char('\n');

    return cmd;
}

void OctaveExpression::parseOutput(const QString& output)
{
    if (output.size() > 200)
        qDebug() << "parseOutput: " << output.left(200) << "...";
    else
        qDebug() << "parseOutput: " << output;

    if (!output.trimmed().isEmpty())
    {
        // TODO: what about help in comment? printf with '... help ...'?
        // This must be corrected.
        if (command().contains(QLatin1String("help")))
            addResult(new Cantor::HelpResult(output));
        else
            addResult(new Cantor::TextResult(output));
    }

    m_finished = true;
    if (!m_plotPending)
        setStatus(Done);
}

void OctaveExpression::parseError(const QString& error)
{
    if (error.startsWith(QLatin1String("warning: ")))
    {
        // It's warning, so add as result
        addResult(new Cantor::TextResult(error));
    }
    else
    {
        setErrorMessage(error);
        setStatus(Error);
    }
}

void OctaveExpression::imageChanged()
{
    QFile file(m_plotFilename);
    if(!file.open(QIODevice::ReadOnly)/* || file.size() <= 0*/)
    {
        m_plotPending = false;
        setResult(new Cantor::TextResult(i18n("Invalid image file generated.")));
        setStatus(Error);
        return;
    }

    const QUrl& url = QUrl::fromLocalFile(m_plotFilename);
    auto* newResult = new Cantor::ImageResult(url);

    bool found = false;
    for (int i = 0; i < results().size(); i++)
        if (results()[i]->type() == newResult->type())
        {
            replaceResult(i, newResult);
            found = true;
        }

    if (!found)
        addResult(newResult);

    m_plotPending = false;

    if (m_finished && status() == Expression::Computing)
        setStatus(Done);
}
