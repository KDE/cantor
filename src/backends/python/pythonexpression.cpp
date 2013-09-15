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
    Copyright (C) 2012 Filipe Saraiva <filipe@kde.org>
 */

#include "pythonexpression.h"

#include <config-cantorlib.h>

#include "textresult.h"
#include "imageresult.h"
#include "helpresult.h"
#include <kdebug.h>
#include <kiconloader.h>
#include <QFile>
#include "pythonsession.h"
#include "settings.h"

#include "imageresult.h"
#include <qdir.h>
typedef Cantor::ImageResult PythonPlotResult;

PythonExpression::PythonExpression( Cantor::Session* session ) : Cantor::Expression(session)
{
    kDebug() << "PythonExpression construtor";
}

PythonExpression::~PythonExpression()
{

}

void PythonExpression::evaluate()
{
    kDebug() << "evaluating " << command();
    setStatus(Cantor::Expression::Computing);

    PythonSession* pythonSession = dynamic_cast<PythonSession*>(session());

    kDebug() << PythonSettings::integratePlots() << command().contains("show()");

    if((PythonSettings::integratePlots()) && (command().contains("show()"))){

        kDebug() << "Preparing export figures property";

        QString saveFigCommand = "savefig('cantor-export-python-figure-%1.png')";

        setCommand(command().replace("show()", saveFigCommand.arg(rand())));

        kDebug() << "New Command " << command();

    }

    pythonSession->runExpression(this);
}

void PythonExpression::parseOutput(QString output)
{
    kDebug() << "output: " << output;

    if(command().contains("help(")){
        setResult(new Cantor::HelpResult(output.remove(output.lastIndexOf("None"), 4)));
    } else {
        setResult(new Cantor::TextResult(output));
    }

    setStatus(Cantor::Expression::Done);
}

void PythonExpression::parseError(QString error)
{
    kDebug() << "error: " << error;
    setResult(new Cantor::TextResult(error));

    setStatus(Cantor::Expression::Error);
}

void PythonExpression::parsePlotFile(QString filename)
{
    kDebug() << "parsePlotFile";

    kDebug() << "PythonExpression::parsePlotFile: " << filename;

    setResult(new PythonPlotResult(filename));

    setPlotPending(false);

    if (m_finished)
    {
        kDebug() << "PythonExpression::parsePlotFile: done";
        setStatus(Done);
    }
}

void PythonExpression::interrupt()
{
    kDebug()<<"interruptinging command";
    setStatus(Cantor::Expression::Interrupted);
}

void PythonExpression::setPlotPending(bool plot)
{
    m_plotPending = plot;
}
