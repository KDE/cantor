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
#include <QFileSystemWatcher>
#include <QTemporaryFile>

PythonExpression::PythonExpression(Cantor::Session* session, bool internal) : Cantor::Expression(session, internal),
    m_tempFile(nullptr)
{
}

PythonExpression::~PythonExpression() {
    if(m_tempFile)
        delete m_tempFile;
}

void PythonExpression::evaluate()
{
    if(m_tempFile) {
        delete m_tempFile;
        m_tempFile = nullptr;
    }

    session()->enqueueExpression(this);
}

QString PythonExpression::internalCommand()
{
    QString cmd = command();

    PythonSession* pythonSession = static_cast<PythonSession*>(session());
    if((pythonSession->integratePlots()) && (command().contains(QLatin1String("show()")))){
        m_tempFile = new QTemporaryFile(QDir::tempPath() + QLatin1String("/cantor_python-XXXXXX.png"));
        m_tempFile->open();
        QString saveFigCommand = QLatin1String("savefig('%1')");
        cmd.replace(QLatin1String("show()"), saveFigCommand.arg(m_tempFile->fileName()));

        QFileSystemWatcher* watcher = fileWatcher();
        watcher->removePaths(watcher->files());
        watcher->addPath(m_tempFile->fileName());
        connect(watcher, &QFileSystemWatcher::fileChanged, this, &PythonExpression::imageChanged,  Qt::UniqueConnection);
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

    if(command().simplified().startsWith(QLatin1String("help("))){
        setResult(new Cantor::HelpResult(output.remove(output.lastIndexOf(QLatin1String("None")), 4)));
    } else {
        if (!output.isEmpty())
            addResult(new Cantor::TextResult(output));
    }

    if (m_tempFile == nullptr || result() != nullptr) // not plot expression
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
        addResult(new Cantor::TextResult(warning));
}

void PythonExpression::imageChanged()
{
    if(m_tempFile->size() <= 0)
        return;

    Cantor::ImageResult* newResult = new Cantor::ImageResult(QUrl::fromLocalFile(m_tempFile->fileName()));
    if (result() == nullptr)
        setResult(newResult);
    else
    {
        bool found = false;
        for (int i = 0; i < results().size(); i++)
            if (results()[i]->type() == newResult->type())
            {
                replaceResult(i, newResult);
                found = true;
            }
        if (!found)
            addResult(newResult);
    }
    setStatus(Done);
}

void PythonExpression::interrupt()
{
    qDebug()<<"interruptinging command";
    setStatus(Cantor::Expression::Interrupted);
}
