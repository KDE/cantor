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

#include "python2expression.h"

#include <config-cantorlib.h>

#include "textresult.h"
#include "imageresult.h"
#include "helpresult.h"
#include <kdebug.h>
#include <kiconloader.h>
#include <QFile>
#include "python2session.h"
#include "settings.h"

#include "imageresult.h"
#include <qdir.h>
typedef Cantor::ImageResult Python2PlotResult;

Python2Expression::Python2Expression( Cantor::Session* session ) : Cantor::Expression(session)
{
    kDebug() << "Python2Expression construtor";
}

Python2Expression::~Python2Expression()
{

}

void Python2Expression::evaluate()
{
    kDebug() << "evaluating " << command();
    setStatus(Cantor::Expression::Computing);

    Python2Session* pythonSession = dynamic_cast<Python2Session*>(session());

    kDebug() << Python2Settings::integratePlots() << command().contains("show()");

    if((Python2Settings::integratePlots()) && (command().contains("show()"))){

        kDebug() << "Preparing export figures property";

        QString saveFigCommand = "savefig('cantor-export-python-figure-%1.png')";

        setCommand(command().replace("show()", saveFigCommand.arg(rand())));

        kDebug() << "New Command " << command();

    }

    pythonSession->runExpression(this);
}

void Python2Expression::parseOutput(QString output)
{
    kDebug() << "output: " << output;

    if(command().simplified().startsWith("help(")){
        setResult(new Cantor::HelpResult(output.remove(output.lastIndexOf("None"), 4)));
    } else {
        setResult(new Cantor::TextResult(output));
    }

    setStatus(Cantor::Expression::Done);
}

void Python2Expression::parseError(QString error)
{
    kDebug() << "error: " << error;
    setErrorMessage(error.replace("\n", "<br>"));

    setStatus(Cantor::Expression::Error);
}

void Python2Expression::parsePlotFile(QString filename)
{
    kDebug() << "parsePlotFile";

    kDebug() << "Python2Expression::parsePlotFile: " << filename;

    setResult(new Python2PlotResult(filename));

    setPlotPending(false);

    if (m_finished)
    {
        kDebug() << "Python2Expression::parsePlotFile: done";
        setStatus(Done);
    }
}

void Python2Expression::interrupt()
{
    kDebug()<<"interruptinging command";
    setStatus(Cantor::Expression::Interrupted);
}

void Python2Expression::setPlotPending(bool plot)
{
    m_plotPending = plot;
}
