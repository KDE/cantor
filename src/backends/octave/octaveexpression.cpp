/*
    Copyright (C) 2010 Miha Čančula <miha.cancula@gmail.com>

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
*/

#include "octaveexpression.h"
#include "octavesession.h"
#include "defaultvariablemodel.h"

#include "textresult.h"

#include <QDebug>
#include <QFile>
#include <helpresult.h>


static const QLatin1String printCommand("cantor_print();");

OctaveExpression::OctaveExpression(Cantor::Session* session, bool internal): Expression(session, internal),
    m_plotPending(false),
    m_finished(false)
{
    m_plotCommands << QLatin1String("plot") << QLatin1String("semilogx") << QLatin1String("semilogy") << QLatin1String("loglog")
                   << QLatin1String("polar") << QLatin1String("contour") << QLatin1String("bar")
                   << QLatin1String("stairs") << QLatin1String("errorbar")  << QLatin1String("sombrero")
                   << QLatin1String("hist") << QLatin1String("fplot") << QLatin1String("imshow")
                   << QLatin1String("stem") << QLatin1String("stem3") << QLatin1String("scatter") << QLatin1String("pareto") << QLatin1String("rose")
                   << QLatin1String("pie") << QLatin1String("quiver") << QLatin1String("compass") << QLatin1String("feather")
                   << QLatin1String("pcolor") << QLatin1String("area") << QLatin1String("fill") << QLatin1String("comet")
                   << QLatin1String("plotmatrix")
                   /* 3d-plots */
                   << QLatin1String("plot3")
                   << QLatin1String("mesh") << QLatin1String("meshc") << QLatin1String("meshz")
                   << QLatin1String("surf") << QLatin1String("surfc") << QLatin1String("surfl") << QLatin1String("surfnorm")
                   << QLatin1String("isosurface")<< QLatin1String("isonormals") << QLatin1String("isocaps")
                   /* 3d-plots defined by a function */
                   << QLatin1String("ezplot3") << QLatin1String("ezmesh") << QLatin1String("ezmeshc") << QLatin1String("ezsurf") << QLatin1String("ezsurfc");
    m_plotCommands << QLatin1String("cantor_plot2d") << QLatin1String("cantor_plot3d");
}

void OctaveExpression::interrupt()
{
    qDebug() << "interrupt";

    setStatus(Interrupted);
}

void OctaveExpression::evaluate()
{
    qDebug() << "evaluate";
    QString cmd = command();
    QStringList cmdWords = cmd.split(QRegExp(QLatin1String("\\b")), QString::SkipEmptyParts);
    if (!cmdWords.contains(QLatin1String("help")) && !cmdWords.contains(QLatin1String("completion_matches")))
    {
        foreach (const QString& plotCmd, m_plotCommands)
        {
            if (cmdWords.contains(plotCmd))
            {
                setPlotPending(true);
                qDebug() << "Executing a plot command";
                break;
            }
        }
    }

    m_finished = false;
    session()->enqueueExpression(this);
}

QString OctaveExpression::internalCommand()
{
    QString cmd = command();

    if (m_plotPending && !cmd.contains(QLatin1String("cantor_plot")) && !cmd.contains(printCommand))
    {
        if (!cmd.endsWith(QLatin1Char(';')) && !cmd.endsWith(QLatin1Char(',')))
            cmd += QLatin1Char(',');
        cmd += printCommand;
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
            setResult(new Cantor::HelpResult(output));
        }
        else
        {
            setResult(new Cantor::TextResult(output));
        }
    }

    m_finished = true;
    if (!m_plotPending)
        setStatus(Done);
}

void OctaveExpression::parseError(const QString& error)
{
    setErrorMessage(error);
    setStatus(Error);
}

void OctaveExpression::parsePlotFile(const QString& file)
{
    qDebug() << "parsePlotFile";
    if (QFile::exists(file))
    {
        qDebug() << "OctaveExpression::parsePlotFile: " << file;

        setResult(new OctavePlotResult(QUrl::fromLocalFile(file)));
        setPlotPending(false);

        if (m_finished)
        {
            setStatus(Done);
        }
    }
}

void OctaveExpression::setPlotPending(bool plot)
{
    m_plotPending = plot;
}
