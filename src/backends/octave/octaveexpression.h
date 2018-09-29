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

#ifndef OCTAVEEXPRESSION_H
#define OCTAVEEXPRESSION_H

#include <config-cantorlib.h>

#include <expression.h>
#include <QStringList>

#ifdef WITH_EPS
#include "epsresult.h"
using OctavePlotResult = Cantor::EpsResult;
#else
#include "imageresult.h"
typedef Cantor::ImageResult OctavePlotResult;
#endif


class OctaveExpression : public Cantor::Expression
{
    Q_OBJECT

public:
    explicit OctaveExpression(Cantor::Session*, bool internal = false);

    void interrupt() override;
    void evaluate() override;
    void parseOutput(const QString&);
    void parseError(const QString&);
    void parsePlotFile(const QString&);

    void setPlotPending(bool);

private:
    QString m_resultString;
    bool m_plotPending;
    bool m_finished;
    bool m_appendPlotCommand;
    bool m_appendDot;
    QStringList m_plotCommands;
};

#endif // OCTAVEEXPRESSION_H
