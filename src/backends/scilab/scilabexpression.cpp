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
    Copyright (C) 2011 Filipe Saraiva <filip.saraiva@gmail.com>
 */

#include "scilabexpression.h"

#include <config-cantorlib.h>

#include "textresult.h"
#include "imageresult.h"
#include "helpresult.h"
#include <kdebug.h>
#include <kiconloader.h>
#include <QTimer>
#include <QFile>
#include "scilabsession.h"

/*#ifdef WITH_EPS
#include "epsresult.h"
typedef Cantor::EpsResult ScilabPlotResult;
#else*/
#include "imageresult.h"
typedef Cantor::ImageResult ScilabPlotResult;
// #endif

ScilabExpression::ScilabExpression( Cantor::Session* session ) : Cantor::Expression(session)
{
    kDebug() << "ScilabExpression construtor";

    m_plotCommands << "plot"
                   << "plot2d" << "plot2d1" << "plot2d2" << "plot2d3" << "plot2d4"
                   << "plot3d" << "plot3d1" << "plot3d2" << "plot3d3";

    m_timer=new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(evalFinished()));
}

ScilabExpression::~ScilabExpression()
{

}

void ScilabExpression::evaluate()
{
    kDebug() << "evaluating " << command();
    setStatus(Cantor::Expression::Computing);

    ScilabSession* scilabSession = dynamic_cast<ScilabSession*>(session());

    scilabSession->runExpression(this);

    m_timer->start(1000);
}

void ScilabExpression::parseOutput(QString output)
{
    kDebug() << "output: " << output;
    setResult(new Cantor::TextResult(output));
}

void ScilabExpression::parseError(QString error)
{
    kDebug() << "error" << error;
    setResult(new Cantor::TextResult(error));
    setErrorMessage(error);
    setStatus(Cantor::Expression::Error);
}

void ScilabExpression::parsePlotFile(QString file)
{
    kDebug() << "parsePlotFile";
    if (QFile::exists(file))
    {
        kDebug() << "ScilabExpression::parsePlotFile: " << file;

        setResult(new ScilabPlotResult(file));
        setPlotPending(false);

        if (m_finished)
        {
            kDebug() << "ScilabExpression::parsePlotFile: done";
            setStatus(Done);
        }
    }
}

void ScilabExpression::interrupt()
{
    kDebug()<<"interruptinging command";
    m_timer->stop();
    setStatus(Cantor::Expression::Interrupted);
}

void ScilabExpression::evalFinished()
{
    kDebug()<<"evaluation finished";
    setStatus(Cantor::Expression::Done);
}

void ScilabExpression::setPlotPending(bool plot)
{
    m_plotPending = plot;
}
