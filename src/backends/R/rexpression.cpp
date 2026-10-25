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

QString RExpression::formatHelpText(const QString& text)
{
    // Convert R's plain-text help to styled HTML
    // Use semi-transparent backgrounds to work with both light and dark themes
    QString html;
    html.reserve(text.size() * 2);

    const QStringList lines = text.split(QLatin1Char('\n'));
    static const QRegularExpression sectionHeader(QStringLiteral("^([A-Z][A-Za-z ]+):[ ]*$"));
    static const QRegularExpression underlinedHeader(QStringLiteral("^_+([^_]+)_+$"));
    bool inExample = false;

    html += QLatin1String("<div style=\"line-height: 1.4;\">");

    for (int i = 0; i < lines.size(); ++i)
    {
        const QString& line = lines[i];
        QString escaped = line.toHtmlEscaped();

        // Check for section headers like "Description:", "Usage:", "Arguments:", "Examples:"
        auto match = sectionHeader.match(line);
        if (match.hasMatch())
        {
            if (inExample)
            {
                html += QLatin1String("</pre>");
                inExample = false;
            }
            QString section = match.captured(1);
            html += QStringLiteral("<h4 style=\"color: #2277dd; margin: 12px 0 6px 0;\"><b>%1</b></h4>").arg(section.toHtmlEscaped());
            if (section == QLatin1String("Examples") || section == QLatin1String("Usage"))
            {
                html += QLatin1String("<pre style=\"background-color: #d0d0d0; color: #000000; padding: 8px; border-radius: 4px;\">");
                inExample = true;
            }
            continue;
        }

        // Check for underlined headers like _Details_
        match = underlinedHeader.match(line);
        if (match.hasMatch())
        {
            if (inExample)
            {
                html += QLatin1String("</pre>");
                inExample = false;
            }
            html += QStringLiteral("<h4 style=\"color: #2277dd; margin: 12px 0 6px 0;\"><b><i>%1</i></b></h4>").arg(match.captured(1).toHtmlEscaped());
            continue;
        }

        if (inExample)
            html += escaped + QLatin1Char('\n');
        else
        {
            // Regular paragraph text - preserve spacing but allow wrapping
            if (line.trimmed().isEmpty())
                html += QLatin1String("<br/>");
            else
                html += QStringLiteral("<p style=\"margin: 4px 0; white-space: pre-wrap;\">%1</p>").arg(escaped);
        }
    }

    if (inExample)
        html += QLatin1String("</pre>");
    html += QLatin1String("</div>");
    return html;
}

void RExpression::evaluate()
{
    QString cmd = command();

    // help.start() launches R's HTTP server which doesn't work properly in embedded mode
    if (cmd.trimmed() == QStringLiteral("help.start()") || cmd.trimmed() == QStringLiteral("help.start ()"))
    {
        setErrorMessage(i18n("help.start() is not supported in embedded R mode. Use help(topic) or ?topic instead."));
        setStatus(Cantor::Expression::Error);
        return;
    }

    //check whether we need to interpret the current command as a help command.
    //see https://www.r-project.org/help.html for the list of different ways to get help in R.
    if(cmd.startsWith(QLatin1Char('?')) || cmd.startsWith(QStringLiteral("help("))
        || cmd.startsWith(QStringLiteral("apropos("))
        || cmd.startsWith(QStringLiteral("vignette("))
        || cmd == QStringLiteral("demos()")
        || cmd.startsWith(QStringLiteral("help.search(")) )
        setIsHelpRequest(true);

    setCommand(cmd);
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
            addResult(new Cantor::HelpResult(formatHelpText(text), true));
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
                addResult(new Cantor::HelpResult(formatHelpText(content), true));
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
