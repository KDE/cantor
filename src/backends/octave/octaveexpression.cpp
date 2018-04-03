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

#include <config-cantorlib.h>


static const char* printCommand = "cantor_print();";

OctaveExpression::OctaveExpression(Cantor::Session* session): Expression(session),
    m_plotPending(false),
    m_finished(false),
    m_error(false)
{
    m_plotCommands << QLatin1String("plot") << QLatin1String("semilogx") << QLatin1String("semilogy") << QLatin1String("loglog")
                   << QLatin1String("polar") << QLatin1String("mesh") << QLatin1String("contour") << QLatin1String("bar")
                   << QLatin1String("stairs") << QLatin1String("errorbar") << QLatin1String("surf") << QLatin1String("sombrero")
                   << QLatin1String("hist") << QLatin1String("fplot") << QLatin1String("imshow");
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
    if ( m_plotPending && !cmd.contains(QLatin1String("cantor_plot")) && !cmd.contains(QLatin1String(printCommand)))
    {
        // This was a manual plot, we have to add a print command
        if (!cmd.endsWith(QLatin1Char(';')) && !cmd.endsWith(QLatin1Char(',')))
        {
            cmd += QLatin1Char(',');
        }
        cmd += QLatin1String(printCommand);
        setCommand(cmd);
    }
    m_finished = false;
    setStatus(Computing);
    OctaveSession* octaveSession = dynamic_cast<OctaveSession*>(session());
    if (octaveSession)
    {
        octaveSession->runExpression(this);
    }
}


void OctaveExpression::parseOutput ( QString output )
{
    qDebug() << "parseOutput: " << output;
    m_resultString += output;
    if (!m_resultString.trimmed().isEmpty())
    {
        if (command().contains(QLatin1String("help")))
        {
            setResult(new Cantor::HelpResult(m_resultString));
        }
        else
        {
            setResult(new Cantor::TextResult(m_resultString));
        }
    }
}

void OctaveExpression::parseError(QString error)
{
    qDebug() << error;
    if (false && error.contains(QLatin1String("warning")))
    {
        parseOutput(error);
    }
    else
    {
        m_error = true;
        setErrorMessage(error);
        setStatus(Error);
    }
}

void OctaveExpression::parsePlotFile(QString file)
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

void OctaveExpression::finalize()
{
    qDebug() << "finalize: " << m_resultString;
    foreach ( const QString& line, m_resultString.simplified().split(QLatin1Char('\n'), QString::SkipEmptyParts) )
    {
        if ((m_resultString.contains(QLatin1Char('='))) && !(command().startsWith(QLatin1String("help(")))
                && !(command().contains(QLatin1String("help "))) && !(command().contains(QLatin1String("type("))))
        {
            qDebug() << line;
            // Probably a new variable
            QStringList parts = line.split(QLatin1Char('='));
            if (parts.size() >= 2)
            {
                Cantor::DefaultVariableModel* model = dynamic_cast<Cantor::DefaultVariableModel*>(session()->variableModel());
                if (model)
                {
                    model->addVariable(parts.first().trimmed(), parts.last().trimmed());
                }
            }
        }
    }
    qDebug() << m_plotPending << m_error;
    m_finished = true;
    if (!m_plotPending)
    {
        setStatus(m_error ? Error : Done);
    }
}
void OctaveExpression::setPlotPending(bool plot)
{
    m_plotPending = plot;
}
