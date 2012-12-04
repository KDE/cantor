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
#include <QTimer>
#include <QFile>
#include "pythonsession.h"
#include "settings.h"

#include "imageresult.h"
#include <qdir.h>
// typedef Cantor::ImageResult PythonPlotResult;

PythonExpression::PythonExpression( Cantor::Session* session ) : Cantor::Expression(session)
{
    kDebug() << "PythonExpression construtor";

    m_timer=new QTimer(this);
    m_timer->setSingleShot(true);

    connect(m_timer, SIGNAL(timeout()), this, SLOT(evalFinished()));
}

PythonExpression::~PythonExpression()
{

}

void PythonExpression::evaluate()
{
    kDebug() << "evaluating " << command();
    setStatus(Cantor::Expression::Computing);

    PythonSession* pythonSession = dynamic_cast<PythonSession*>(session());

//     if((PythonSettings::integratePlots()) && (command().contains("plot"))){
//
//         kDebug() << "Preparing export figures property";
//
//         QString exportCommand;
//
//         QStringList commandList = command().split("\n");
//
//         for(int count = 0; count < commandList.size(); count++){
//
//             if(commandList.at(count).toLocal8Bit().contains("plot")){
//
//                 exportCommand = QString("\nxs2png(gcf(), 'cantor-export-scilab-figure-%1.png');\ndelete(gcf());").arg(rand());
//
//                 commandList[count].append(exportCommand);
//
//                 exportCommand.clear();
//             }
//
//             kDebug() << "Command " << count << ": " << commandList.at(count).toLocal8Bit().constData();
//         }
//
//         QString newCommand = commandList.join("\n");
//         newCommand.prepend("clf();\n");
//         newCommand.append("\n");
//
//         this->setCommand(newCommand);
//
//         kDebug() << "New Command " << command();
//
//     }

    pythonSession->runExpression(this);

//     m_timer->start(1000);
}

void PythonExpression::parseOutput(QString output)
{
    kDebug() << "output: " << output;
    setResult(new Cantor::TextResult(output));
}

// void PythonExpression::parsePlotFile(QString filename)
// {
//     kDebug() << "parsePlotFile";
//
//     kDebug() << "PythonExpression::parsePlotFile: " << filename;
//
//     setResult(new PythonPlotResult(filename));
//
//     setPlotPending(false);
//
//     if (m_finished)
//     {
//         kDebug() << "PythonExpression::parsePlotFile: done";
//         setStatus(Done);
//     }
// }

void PythonExpression::interrupt()
{
    kDebug()<<"interruptinging command";
//     m_timer->stop();
    setStatus(Cantor::Expression::Interrupted);
}

void PythonExpression::evalFinished()
{
    kDebug()<<"evaluation finished";
    setStatus(Cantor::Expression::Done);
}

// void PythonExpression::setPlotPending(bool plot)
// {
//     m_plotPending = plot;
// }
