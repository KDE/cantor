/*
    SPDX-FileCopyrightText: 2010 Miha Čančula <miha.cancula@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "octaveexpression.h"
#include "octavesession.h"
#include "defaultvariablemodel.h"

#include "textresult.h"
#include "epsresult.h"
#include "imageresult.h"

#include <QDebug>
#include <QFile>
#include <QDir>
#include <QFileSystemWatcher>
#include <QRegularExpression>
#include <QTemporaryFile>
#include <helpresult.h>

#include "settings.h"


static const QString printCommandTemplate = QString::fromLatin1("cantor_print('%1', '%2');");
const QStringList OctaveExpression::plotExtensions({
#ifdef WITH_EPS
    QLatin1String("eps"),
#endif
    QLatin1String("png"),
    QLatin1String("svg"),
    QLatin1String("jpeg")
});

OctaveExpression::OctaveExpression(Cantor::Session* session, bool internal): Expression(session, internal)
{
}

OctaveExpression::~OctaveExpression()
{
}

void OctaveExpression::interrupt()
{
    qDebug() << "interrupt";

    setStatus(Interrupted);
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

    OctaveSession* octaveSession = static_cast<OctaveSession*>(session());
    if (octaveSession->isIntegratedPlotsEnabled() && !session()->enabledGraphicPackages().isEmpty() && !isInternal())
    {
        QStringList cmdWords = cmd.split(QRegularExpression(QStringLiteral("\\b")), QString::SkipEmptyParts);
        if (!cmdWords.contains(QLatin1String("help")) && !cmdWords.contains(QLatin1String("completion_matches")))
        {
            Q_ASSERT(session()->enabledGraphicPackages().size() == 1);
            const Cantor::GraphicPackage& package = session()->enabledGraphicPackages().first();
            Q_ASSERT(package.id() == QLatin1String("octave_universal"));
            for (const QString& plotCmd : package.plotCommandPrecentsKeywords())
                if (cmdWords.contains(plotCmd))
                {
                    if (package.isHavePlotCommand())
                    {
                        m_plotFilename = octaveSession->plotFilePrefixPath() + QString::number(id()) + QLatin1String(".") + plotExtensions[OctaveSettings::inlinePlotFormat()];

                        if (!cmd.endsWith(QLatin1Char(';')) && !cmd.endsWith(QLatin1Char(',')))
                            cmd += QLatin1Char(',');
                        cmd.append(package.savePlotCommand(octaveSession->plotFilePrefixPath(), id(), plotExtensions[OctaveSettings::inlinePlotFormat()]));

                        QFileSystemWatcher* watcher = fileWatcher();
                        if (!watcher->files().isEmpty())
                            watcher->removePaths(watcher->files());

                        // Add path works only with existed paths, so create the file
                        QFile file(m_plotFilename);
                        file.open(QFile::WriteOnly);
                        file.close();
                        watcher->addPath(m_plotFilename);
                        m_plotPending = true;

                        connect(watcher, &QFileSystemWatcher::fileChanged, this, &OctaveExpression::imageChanged,  Qt::UniqueConnection);
                    }
                    break;
                }
        }
    }

    // We need remove all comments here, because below we merge all strings to one long string
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
    qDebug() << "parseOutput: " << output;

    if (!output.trimmed().isEmpty())
    {
        // TODO: what about help in comment? printf with '... help ...'?
        // This must be corrected.
        if (command().contains(QLatin1String("help")))
        {
            addResult(new Cantor::HelpResult(output));
        }
        else
        {
            addResult(new Cantor::TextResult(output));
        }
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
    if(QFile(m_plotFilename).size() <= 0)
        return;

    const QUrl& url = QUrl::fromLocalFile(m_plotFilename);
    Cantor::Result* newResult;
    if (m_plotFilename.endsWith(QLatin1String(".eps")))
        newResult = new Cantor::EpsResult(url);
    else
        newResult = new Cantor::ImageResult(url);

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
