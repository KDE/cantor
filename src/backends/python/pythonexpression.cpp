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
#include <QDebug>
#include <KIconLoader>
#include <QFile>
#include "pythonsession.h"
#include "settings.h"

#include <QDir>
typedef Cantor::ImageResult PythonPlotResult;

PythonExpression::PythonExpression(Cantor::Session* session) : Cantor::Expression(session)
{
    qDebug() << "PythonExpression construtor";
}

void PythonExpression::evaluate()
{
    setStatus(Cantor::Expression::Computing);

    PythonSession* pythonSession = dynamic_cast<PythonSession*>(session());

    qDebug() << pythonSession->integratePlots() << command().contains(QLatin1String("show()"));

    if((pythonSession->integratePlots()) && (command().contains(QLatin1String("show()")))){

        qDebug() << "Preparing export figures property";

        QString saveFigCommand = QLatin1String("savefig('cantor-export-python-figure-%1.png')");

        setCommand(command().replace(QLatin1String("show()"), saveFigCommand.arg(rand())));

        qDebug() << "New Command " << command();

    }

    pythonSession->runExpression(this);
}

void PythonExpression::parseOutput(QString output)
{
    qDebug() << "output: " << output;

    if(command().simplified().startsWith(QLatin1String("help("))){
        setResult(new Cantor::HelpResult(output.remove(output.lastIndexOf(QLatin1String("None")), 4)));
    } else {
        setResult(new Cantor::TextResult(output));
    }

    setStatus(Cantor::Expression::Done);
}

void PythonExpression::parseError(QString error)
{
    qDebug() << "error: " << error;
    setErrorMessage(error.replace(QLatin1String("\n"), QLatin1String("<br>")));

    setStatus(Cantor::Expression::Error);
}

void PythonExpression::parsePlotFile(const QString& filename)
{
    qDebug() << "parsePlotFile";

    qDebug() << "PythonExpression::parsePlotFile: " << filename;

    setResult(new PythonPlotResult(QUrl::fromLocalFile(filename)));

    setPlotPending(false);

    if (m_finished)
    {
        qDebug() << "PythonExpression::parsePlotFile: done";
        setStatus(Done);
    }
}

void PythonExpression::interrupt()
{
    qDebug()<<"interruptinging command";
    setStatus(Cantor::Expression::Interrupted);
}

void PythonExpression::setPlotPending(bool plot)
{
    m_plotPending = plot;
}
