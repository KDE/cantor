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

    // handle matplotlib's show() command
    if((PythonSettings::integratePlots()) && (command().contains(QLatin1String("show()"))))
    {
        QString extension;
        if (PythonSettings::inlinePlotFormat() == 0)
            extension = QLatin1String("pdf");
        else if (PythonSettings::inlinePlotFormat() == 1)
            extension = QLatin1String("svg");
        else if (PythonSettings::inlinePlotFormat() == 2)
            extension = QLatin1String("png");

        m_tempFile = new QTemporaryFile(QDir::tempPath() + QLatin1String("/cantor_python-XXXXXX.%1").arg(extension));
        m_tempFile->open();
        QString saveFigCommand = QLatin1String("savefig('%1')");
        cmd.replace(QLatin1String("show()"), saveFigCommand.arg(m_tempFile->fileName()));

        // set the plot size in inches
        // TODO: matplotlib is usually imported via "import matplotlib.pyplot as plt" and we set
        // plot size for plt below but it's still possible to name the module differently and we
        // somehow need to handle such cases, too.
        const double w = PythonSettings::plotWidth() / 2.54;
        const double h = PythonSettings::plotHeight() / 2.54;
        cmd += QLatin1String("\nplt.figure(figsize=(%1, %2))").arg(QString::number(w), QString::number(h));

        QFileSystemWatcher* watcher = fileWatcher();
        watcher->removePaths(watcher->files());
        watcher->addPath(m_tempFile->fileName());
        connect(watcher, &QFileSystemWatcher::fileChanged, this, &PythonExpression::imageChanged,  Qt::UniqueConnection);
    }
    // TODO: handle other plotting frameworks

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

        // ignore IPython's magic commands (https://ipython.readthedocs.io/en/stable/interactive/magics.html):
        // when a Jupytor notebook is loaded, we might get IPython's magics that are not relevant for us and that are also invalid syntax for Python
        // and we need to ignore them if the user wants to execute them again.
        // all these magic commands start with '%' which is invalid syntax in Python.
        // the only exception is "%variables" that we use internally to get the list of variables from the interprepter,
        // s.a. PythonVariableModel::update() and PythonSession::runFirstExpression(). TODO: redesign this part to get rid of % internally.
        if (!command.trimmed().startsWith(QLatin1Char('%')) || command.trimmed().startsWith(QLatin1String("%variables")) )
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
