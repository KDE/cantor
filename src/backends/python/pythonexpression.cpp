/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Filipe Saraiva <filipe@kde.org>
*/

#include "pythonexpression.h"

#include <config-cantorlib.h>

#include "textresult.h"
#include "imageresult.h"
#include "helpresult.h"
#include "session.h"
#include "settings.h"

#include <QDebug>
#include <QDir>
#include <QTemporaryFile>
#include <QFileSystemWatcher>

#include "pythonsession.h"

PythonExpression::PythonExpression(Cantor::Session* session, bool internal) : Cantor::Expression(session, internal)
{
}

PythonExpression::~PythonExpression() {
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

    if((PythonSettings::integratePlots()) && (command().contains(QLatin1String("show()"))))
    {
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

    for(const QString& command : commandLine)
    {
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

void PythonExpression::parseOutput(const QString& output)
{
    qDebug() << "expression output: " << output;
    if(command().simplified().startsWith(QLatin1String("help(")))
    {
        QString resultStr = output;
        setResult(new Cantor::HelpResult(resultStr.remove(output.lastIndexOf(QLatin1String("None")), 4)));
    } else {
        if (!output.isEmpty())
            addResult(new Cantor::TextResult(output));
     }

    setStatus(Cantor::Expression::Done);
}

void PythonExpression::parseError(const QString& error)
{
    qDebug() << "expression error: " << error;
    setErrorMessage(error);

    setStatus(Cantor::Expression::Error);
}

void PythonExpression::parseWarning(const QString& warning)
{
    if (!warning.isEmpty())
    {
        auto* result = new Cantor::TextResult(warning);
        result->setStdErr(true);
        addResult(result);
    }
}

void PythonExpression::imageChanged()
{
    if(m_tempFile->size() <= 0)
        return;

    auto* newResult = new Cantor::ImageResult(QUrl::fromLocalFile(m_tempFile->fileName()));
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
