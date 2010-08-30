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

#include "textresult.h"
#include "epsresult.h"

#include <KDebug>
#include <QtCore/QFile>

static const char* printCommand = "print('-depsc',strcat(tempname(tempdir,'c-ob-'),'.eps'),'-S480,336','-tight');";

OctaveExpression::OctaveExpression(Cantor::Session* session): Expression(session)
{
    m_plotCommands << "plot" << "semilogx" << "semilogy" << "loglog" << "polar"
                   << "mesh" << "contour" << "bar" << "stairs" << "errorbar";
    m_plotCommands << "cantor_plot2d" << "cantor_plot3d";
}


OctaveExpression::~OctaveExpression()
{

}


void OctaveExpression::interrupt()
{
    setStatus(Interrupted);
}

void OctaveExpression::evaluate()
{
    setPlotPending(false);
    QString cmd = command();
    QStringList cmdWords = cmd.split(QRegExp("\\b"), QString::SkipEmptyParts);
    if (!cmdWords.contains("help") && !cmdWords.contains("completion_matches"))
    {
        foreach (const QString& plotCmd, m_plotCommands)
        {
            if (cmdWords.contains(plotCmd))
            {
                setPlotPending(true);
                kDebug() << "Executing a plot command";
                break;
            }
        }
    }
    if ( m_plotPending && !cmd.contains("cantor_plot"))
    {
        // This was a manual plot, we have to add a print command
        if (!cmd.endsWith(';') && !cmd.endsWith(','))
        {
            cmd += ',';
        }
        cmd += printCommand;
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
    m_resultString += output;
    if (!m_resultString.trimmed().isEmpty())
    {
        setResult(new Cantor::TextResult(m_resultString));
    }
}

void OctaveExpression::parseError(QString error)
{
    kDebug() << error;
    // setErrorMessage doesn't preserve Octave's the formatting
    setResult(new Cantor::TextResult(error));
    m_error = true;
}

void OctaveExpression::parseEpsFile(QString file)
{
    if (QFile::exists(file))
    {
        setResult(new Cantor::EpsResult(file));
        setPlotPending(false);
        if (m_finished)
        {
            setStatus(Done);
        }
    }
}

void OctaveExpression::finalize()
{
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
