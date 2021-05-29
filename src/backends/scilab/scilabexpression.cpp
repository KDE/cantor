/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2011 Filipe Saraiva <filipe@kde.org>
*/

#include "scilabexpression.h"

#include <config-cantorlib.h>

#include "textresult.h"
#include "imageresult.h"
#include "helpresult.h"

#include <QDebug>
#include <QDir>
#include <QFile>

#include <KIconLoader>

#include "settings.h"
#include "defaultvariablemodel.h"

using ScilabPlotResult = Cantor::ImageResult;

ScilabExpression::ScilabExpression( Cantor::Session* session, bool internal ) : Cantor::Expression(session, internal),
m_finished(false),
m_plotPending(false)
{
    qDebug() << "ScilabExpression constructor";
}

void ScilabExpression::evaluate()
{
    if((ScilabSettings::integratePlots()) && (command().contains(QLatin1String("plot")))){

        qDebug() << "Preparing export figures property";

        QString exportCommand;

        QStringList commandList = command().split(QLatin1String("\n"));

        for(int count = 0; count < commandList.size(); count++){

            if(commandList.at(count).toLocal8Bit().contains("plot")){

                exportCommand = QString::fromLatin1("\nxs2png(gcf(), 'cantor-export-scilab-figure-%1.png');\ndelete(gcf());").arg(qrand());

                commandList[count].append(exportCommand);

                exportCommand.clear();
            }

            qDebug() << "Command " << count << ": " << commandList.at(count).toLocal8Bit().constData();
        }

        QString newCommand = commandList.join(QLatin1String("\n"));
        newCommand.prepend(QLatin1String("clf();\n"));
        newCommand.append(QLatin1String("\n"));

        this->setCommand(newCommand);

        qDebug() << "New Command " << command();

    }

    session()->enqueueExpression(this);
}

void ScilabExpression::parseOutput(QString output)
{
    qDebug() << "output: " << output;
    const QStringList lines = output.split(QLatin1String("\n"));
    bool isPrefixLines = true;
    for (const QString& line : lines)
    {
        if (isPrefixLines && line.isEmpty())
            continue;

        m_output += line + QLatin1String("\n");
        isPrefixLines = false;
    }

    if (!m_output.simplified().isEmpty())
        setResult(new Cantor::TextResult(m_output));

    evalFinished();
    setStatus(Cantor::Expression::Done);
}

void ScilabExpression::parseError(QString error)
{
    qDebug() << "error" << error;

    setErrorMessage(error);

    evalFinished();
    setStatus(Cantor::Expression::Error);
}

void ScilabExpression::parsePlotFile(QString filename)
{
    qDebug() << "parsePlotFile";

    qDebug() << "ScilabExpression::parsePlotFile: " << filename;

    setResult(new ScilabPlotResult(QUrl::fromLocalFile(filename)));

    setPlotPending(false);

    if (m_finished){
        qDebug() << "ScilabExpression::parsePlotFile: done";
        setStatus(Done);
    }
}

void ScilabExpression::interrupt()
{
    qDebug()<<"interruptinging command";
    setStatus(Cantor::Expression::Interrupted);
}

void ScilabExpression::evalFinished()
{
    qDebug()<<"evaluation finished";

    foreach (const QString& line, m_output.simplified().split(QLatin1Char('\n'), QString::SkipEmptyParts)){
        if (m_output.contains(QLatin1Char('='))){

            qDebug() << line;

            QStringList parts = line.split(QLatin1Char('='));

            if (parts.size() >= 2){
                Cantor::DefaultVariableModel* model = dynamic_cast<Cantor::DefaultVariableModel*>(session()->variableModel());

                if (model){
                    model->addVariable(parts.first().trimmed(), parts.last().trimmed());
                }
            }
        }
    }
}

void ScilabExpression::setPlotPending(bool plot)
{
    m_plotPending = plot;
}
