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
#include "session.h"
#include "settings.h"

#include <KIconLoader>
#include <QDir>
#include <QFileSystemWatcher>
#include <QTemporaryFile>
#include <QFile>
#include <QDebug>

#include "pythonsession.h"

PythonExpression::PythonExpression(Cantor::Session* session, bool internal) : Cantor::Expression(session, internal)
{
}

PythonExpression::~PythonExpression() {
}

void PythonExpression::evaluate()
{
    session()->enqueueExpression(this);
}

QString PythonExpression::internalCommand()
{
    QString cmd = command();

    if (PythonSettings::integratePlots())
    {
        PythonSession* pySession = static_cast<PythonSession*>(session());
        const QString& filepath = pySession->plotFilePrefixPath() + QString::number(pySession->plotFileCounter()) + QLatin1String(".png");

        for(const Cantor::GraphicPackage& package : session()->enabledGraphicPackages())
        {
            if (package.isHavePlotCommand())
            {
                cmd.append(QLatin1String("\n"));
                cmd.append(package.savePlotCommand(filepath, pySession->plotFileCounter()));
            }
        }
    }

    QStringList commandLine = cmd.split(QLatin1String("\n"));

    QString commandProcessing;

    for(const QString& command : commandLine){
        const QString firstLineWord = command.trimmed().replace(QLatin1String("("), QLatin1String(" "))
            .split(QLatin1String(" ")).at(0);

        // Ignore comments
        if (firstLineWord.length() != 0 && firstLineWord[0] == QLatin1Char('#')){

            commandProcessing += command + QLatin1String("\n");
            continue;
        }

        if(firstLineWord.contains(QLatin1String("execfile"))){

            commandProcessing += command;
            continue;
        }

        commandProcessing += command + QLatin1String("\n");

    }

    return commandProcessing;
}

void PythonExpression::parseOutput(QString output)
{
    qDebug() << "expression output: " << output;
    if(command().simplified().startsWith(QLatin1String("help(")))
    {
        setResult(new Cantor::HelpResult(output.remove(output.lastIndexOf(QLatin1String("None")), 4)));
    }
    else if (!output.isEmpty())
    {
        PythonSession* pySession = static_cast<PythonSession*>(session());
        const QString& plotFilePrefixPath = pySession->plotFilePrefixPath();
        const QString& searchPrefixPath = QLatin1String("INNER PLOT INFO CANTOR: ") + plotFilePrefixPath;

        QStringList buffer;
        const QStringList& lines = output.split(QLatin1String("\n"));
        for (const QString& line : lines)
        {
            if (line.startsWith(searchPrefixPath))
            {
                if (!buffer.isEmpty() && !(buffer.size() == 1 && buffer[0].isEmpty()))
                    addResult(new Cantor::TextResult(buffer.join(QLatin1String("\n"))));

                QString filepath = plotFilePrefixPath + QString::number(pySession->plotFileCounter()) + QLatin1String(".png");
                pySession->plotFileCounter()++;
                addResult(new Cantor::ImageResult(QUrl::fromLocalFile(filepath)));
                buffer.clear();
            }
            else
                buffer.append(line);
        }
        if (!buffer.isEmpty() && !(buffer.size() == 1 && buffer[0].isEmpty()))
            addResult(new Cantor::TextResult(buffer.join(QLatin1String("\n"))));
    }

    setStatus(Cantor::Expression::Done);
}

void PythonExpression::parseError(QString error)
{
    qDebug() << "expression error: " << error;
    setErrorMessage(error);

    setStatus(Cantor::Expression::Error);
}

void PythonExpression::parseWarning(QString warning)
{
    if (!warning.isEmpty())
    {
        Cantor::TextResult* result = new Cantor::TextResult(warning);
        result->setStdErr(true);
        addResult(result);
    }
}

void PythonExpression::interrupt()
{
    qDebug()<<"interruptinging command";
    setStatus(Cantor::Expression::Interrupted);
}
