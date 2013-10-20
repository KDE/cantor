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
    Copyright (C) 2011 Filipe Saraiva <filipe@kde.org>
 */

#include "scilabexpression.h"
#include "scilabkeywords.h"

#include <config-cantorlib.h>

#include "textresult.h"
#include "imageresult.h"
#include "helpresult.h"
#include <kdebug.h>
#include <kiconloader.h>
#include <QFile>
#include "scilabsession.h"
#include "settings.h"
#include "defaultvariablemodel.h"

#include "imageresult.h"
#include <qdir.h>

typedef Cantor::ImageResult ScilabPlotResult;

ScilabExpression::ScilabExpression( Cantor::Session* session ) : Cantor::Expression(session)
{
    kDebug() << "ScilabExpression construtor";
}

ScilabExpression::~ScilabExpression()
{

}

void ScilabExpression::evaluate()
{
    kDebug() << "evaluating " << command();
    setStatus(Cantor::Expression::Computing);

    ScilabSession* scilabSession = dynamic_cast<ScilabSession*>(session());

    if((ScilabSettings::integratePlots()) && (command().contains("plot"))){

        kDebug() << "Preparing export figures property";

        QString exportCommand;

        QStringList commandList = command().split("\n");

        for(int count = 0; count < commandList.size(); count++){

            if(commandList.at(count).toLocal8Bit().contains("plot")){

                exportCommand = QString("\nxs2png(gcf(), 'cantor-export-scilab-figure-%1.png');\ndelete(gcf());").arg(rand());

                commandList[count].append(exportCommand);

                exportCommand.clear();
            }

            kDebug() << "Command " << count << ": " << commandList.at(count).toLocal8Bit().constData();
        }

        QString newCommand = commandList.join("\n");
        newCommand.prepend("clf();\n");
        newCommand.append("\n");

        this->setCommand(newCommand);

        kDebug() << "New Command " << command();

    }

    scilabSession->runExpression(this);
}

void ScilabExpression::parseOutput(QString output)
{
    kDebug() << "output: " << output;
    m_output = output;
    setResult(new Cantor::TextResult(output));

    evalFinished();
}

void ScilabExpression::parseError(QString error)
{
    kDebug() << "error" << error;
    setResult(new Cantor::TextResult(error));
    setErrorMessage(error);
    setStatus(Cantor::Expression::Error);

    evalFinished();
}

void ScilabExpression::parsePlotFile(QString filename)
{
    kDebug() << "parsePlotFile";

    kDebug() << "ScilabExpression::parsePlotFile: " << filename;

    setResult(new ScilabPlotResult(filename));

    setPlotPending(false);

    if (m_finished){
        kDebug() << "ScilabExpression::parsePlotFile: done";
        setStatus(Done);
    }
}

void ScilabExpression::interrupt()
{
    kDebug()<<"interruptinging command";
    setStatus(Cantor::Expression::Interrupted);
}

void ScilabExpression::evalFinished()
{
    kDebug()<<"evaluation finished";

    foreach (const QString& line, m_output.simplified().split('\n', QString::SkipEmptyParts)){
        if (m_output.contains('=')){

            kDebug() << line;

            QStringList parts = line.split('=');

            if (parts.size() >= 2){
                Cantor::DefaultVariableModel* model = dynamic_cast<Cantor::DefaultVariableModel*>(session()->variableModel());

                if (model){
                    model->addVariable(parts.first().trimmed(), parts.last().trimmed());
                    ScilabKeywords::instance()->addVariable(parts.first().trimmed());
                }
            }
        }
    }

    setStatus(Cantor::Expression::Done);
}

void ScilabExpression::setPlotPending(bool plot)
{
    m_plotPending = plot;
}
