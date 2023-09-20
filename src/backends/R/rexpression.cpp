/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2018-2022 Alexander Semke <alexander.semke@web.de>
*/

#include "rexpression.h"

#include "textresult.h"
#include "imageresult.h"
#include "helpresult.h"
#include "epsresult.h"
#include "rsession.h"

#include <QDebug>
#include <KLocalizedString>
#include <QMimeType>
#include <QMimeDatabase>
#include <QFile>

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
    qDebug()<<"interrupting command";
    setStatus(Cantor::Expression::Interrupted);
}

void RExpression::parseOutput(const QString& text)
{
    //qDebug() << "output text: " << text;
    if (!text.trimmed().isEmpty())
    {
        if(isHelpRequest())
            addResult(new Cantor::HelpResult(text));
        else
            addResult(new Cantor::TextResult(text));
    }
    setStatus(Cantor::Expression::Done);
}

void RExpression::parseError(const QString& text)
{
    qDebug() << "error text: " << text;
    setErrorMessage(text);
    setStatus(Cantor::Expression::Error);
}

void RExpression::addInformation(const QString& information)
{
    static_cast<RSession*>(session())->sendInputToServer(information);
}

void RExpression::showFilesAsResult(const QStringList& files)
{
    qDebug()<<"showing files: "<<files;
    for (const QString& file : files)
    {
        QMimeDatabase db;
        auto type = db.mimeTypeForUrl(QUrl(file));
        qDebug()<<"MimeType: "<<type.name();
        if(type.name() == QLatin1String("application/pdf"))
        {
            setResult(new Cantor::ImageResult(QUrl::fromLocalFile(file)));
            setStatus(Cantor::Expression::Done);
        }
        else
            if (type.name().contains(QLatin1String("image")))
        {
            setResult(new Cantor::ImageResult(QUrl::fromLocalFile(file)));
            setStatus(Cantor::Expression::Done);
        }
        else if(type.inherits(QLatin1String("text/plain"))
            || type.inherits(QLatin1String("application/x-extension-html"))
            ||type.inherits(QLatin1String("application/octet-stream")) )
        {
            //Htmls are also plain texts, combining this in one
            const bool isHtml = type.inherits(QLatin1String("text/html"))
                || type.inherits(QLatin1String("application/x-extension-html"))
                || type.inherits(QLatin1String("application/octet-stream"));
            if(isHtml)
                qDebug()<<"it's a HTML document";
            else
                qDebug()<<"it's a plain text";

            QFile f(file);
            if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                setResult(new Cantor::TextResult(i18n("Error opening file %1", file)));
                setErrorMessage(i18n("Error opening file %1", file));
                setStatus(Cantor::Expression::Error);
            }
            QString content=QTextStream(&f).readAll();
            if (!isHtml)
            {
                //Escape whitespace
                content.replace( QLatin1Char(' '), QLatin1String("&nbsp;"));
                //replace appearing backspaces, as they mess the whole output up
                content.remove(QRegExp(QLatin1String(".\b")));
            }
            else
                content.remove(QLatin1String("_\b"));

            qDebug()<<"content: "<<content;
            if(isHelpRequest())
                setResult(new Cantor::HelpResult(content));
            else
                setResult(new Cantor::TextResult(content));
            setStatus(Cantor::Expression::Done);
        }
        else
        {
            // File has unsupported mime type, but we suspect, that it is text, so we open the file in the script editor
            // Even if it's not text, the script editor can deal with it.
            setStatus(Cantor::Expression::Done);
            const QString& editor = QStandardPaths::findExecutable(QLatin1String("cantor_scripteditor"));
            int code = QProcess::execute(editor, QStringList(file));
            if (code == -2)
                qDebug() << "failed to open the file " << file << " with the script editor '" << editor << "'";
            else if (code == -1)
                qDebug() << "Cantor script editor crashed";
        }
    }
}
