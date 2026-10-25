/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2018-2022 Alexander Semke <alexander.semke@web.de>
*/

#include "rexpression.h"

#include "textresult.h"
#include "imageresult.h"
#include "helpresult.h"
#include "rsession.h"

#include <QDebug>
#include <QProcess>
#include <KLocalizedString>
#include <QMimeType>
#include <QMimeDatabase>
#include <QRegularExpression>
#include <QFile>
#include <QImage>

RExpression::RExpression( Cantor::Session* session, bool internal ) : Cantor::Expression(session, internal)
{

}

void RExpression::evaluate()
{
    const auto& cmd = command();

    //check whether we need to interpret the current command as a help command.
    //see https://www.r-project.org/help.html for the list of different ways to get help in R.
    if(cmd.startsWith(QLatin1Char('?')) || cmd.startsWith(QStringLiteral("help("))
        || cmd.startsWith(QStringLiteral("apropos("))
        || cmd.startsWith(QStringLiteral("vignette("))
        || cmd == QStringLiteral("demos()")
        || cmd.startsWith(QStringLiteral("help.search(")) )
        setIsHelpRequest(true);

    session()->enqueueExpression(this);
}

void RExpression::interrupt()
{
    setStatus(Cantor::Expression::Interrupted);
}

void RExpression::parseOutput(const QString& text)
{
    // If we already showed partial output, just finalize with the complete text
    if (m_partialResult)
    {
        m_partialResult->setData(text);
        m_partialResult = nullptr;
        m_partialOutput.clear();
    }
    else if (!text.trimmed().isEmpty())
    {
        if(isHelpRequest())
            addResult(new Cantor::HelpResult(text));
        else
            addResult(new Cantor::TextResult(text));
    }
    setStatus(Cantor::Expression::Done);
}

void RExpression::parsePartialOutput(const QString& text)
{
    if (text.isEmpty())
        return;

    m_partialOutput += text;

    if (!m_partialResult)
    {
        m_partialResult = new Cantor::TextResult(m_partialOutput);
        addResult(m_partialResult);
    }
    else
    {
        m_partialResult->setData(m_partialOutput);
        Q_EMIT gotResult();  // Signal UI to refresh
    }
}

void RExpression::addInformation(const QString& information)
{
    static_cast<RSession*>(session())->sendInputToServer(information);
}

void RExpression::showFilesAsResult(const QStringList& files)
{
    for (const QString& file : files)
    {
        QMimeDatabase db;
        auto type = db.mimeTypeForUrl(QUrl(file));
        if(type.name() == QLatin1String("application/pdf"))
        {
            auto* result = new Cantor::ImageResult(QUrl::fromLocalFile(file));
            if (result->data().value<QImage>().isNull()) {
                delete result;
                continue;
            }
            addResult(result);
        }
        else if (type.name().contains(QLatin1String("image")))
        {
            auto* result = new Cantor::ImageResult(QUrl::fromLocalFile(file));
            if (result->data().value<QImage>().isNull()) {
                delete result;
                continue;
            }
            addResult(result);
        }
        else if(type.inherits(QLatin1String("text/plain"))
            || type.inherits(QLatin1String("application/x-extension-html"))
            ||type.inherits(QLatin1String("application/octet-stream")) )
        {
            const bool isHtml = type.inherits(QLatin1String("text/html"))
                || type.inherits(QLatin1String("application/x-extension-html"))
                || type.inherits(QLatin1String("application/octet-stream"));

            QFile f(file);
            if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                addResult(new Cantor::TextResult(i18n("Error opening file %1", file)));
                setErrorMessage(i18n("Error opening file %1", file));
                continue;
            }
            QString content=QTextStream(&f).readAll();
            if (!isHtml)
            {
                content.replace( QLatin1Char(' '), QLatin1String("&nbsp;"));
                content.remove(QRegularExpression(QLatin1String(".\b")));
            }
            else
                content.remove(QLatin1String("_\b"));

            if(isHelpRequest())
                addResult(new Cantor::HelpResult(content));
            else
                addResult(new Cantor::TextResult(content));
        }
        else
        {
            const QString& editor = QStandardPaths::findExecutable(QLatin1String("cantor_scripteditor"));
            QProcess::execute(editor, QStringList(file));
        }
    }
}
