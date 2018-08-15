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
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
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
#include <QStringList>
#include <QTextDocument>

RExpression::RExpression( Cantor::Session* session ) : Cantor::Expression(session)
{

}

RExpression::~RExpression()
{

}

void RExpression::evaluate()
{
    if(command().startsWith(QLatin1Char('?')))
        m_isHelpRequest=true;
    else
        m_isHelpRequest=false;

    session()->enqueueExpression(this);
}

void RExpression::interrupt()
{
    qDebug()<<"interrupting command";
    if(status()==Cantor::Expression::Computing)
        session()->interrupt();
    setStatus(Cantor::Expression::Interrupted);
}

void RExpression::finished(int returnCode, const QString& text)
{
    if(returnCode==RExpression::SuccessCode)
    {
        qDebug() << "text: " << text;
        setResult(new Cantor::TextResult(text));
        setStatus(Cantor::Expression::Done);
    }else if (returnCode==RExpression::ErrorCode)
    {
        qDebug() << "text: " << text;
        //setResult(new Cantor::TextResult(text));
        setErrorMessage(text);
        setStatus(Cantor::Expression::Error);
    }
}

void RExpression::evaluationStarted()
{
    setStatus(Cantor::Expression::Computing);
}

void RExpression::addInformation(const QString& information)
{
    static_cast<RSession*>(session())->sendInputToServer(information);
}

void RExpression::showFilesAsResult(const QStringList& files)
{
    qDebug()<<"showing files: "<<files;
    foreach(const QString& file, files)
    {
        QMimeType type;
        QMimeDatabase db;

        type=db.mimeTypeForUrl(QUrl(file));
        qDebug()<<"MimeType: "<<type.name();
        if(type.inherits(QLatin1String("application/postscript")))
        {
            qDebug()<<"its PostScript";
            setResult(new Cantor::EpsResult(QUrl::fromLocalFile(file)));
        }
        else if(type.inherits(QLatin1String("text/plain"))
            || type.inherits(QLatin1String("application/x-extension-html")))
        {
            //Htmls are also plain texts, combining this in one
            const bool isHtml = type.inherits(QLatin1String("text/html"))
                || type.inherits(QLatin1String("application/x-extension-html"));
            if(isHtml)
                qDebug()<<"its a HTML document";
            else
                qDebug()<<"its plain text";

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

            qDebug()<<"content: "<<content;
            if(m_isHelpRequest)
                setResult(new Cantor::HelpResult(content));
            else
                setResult(new Cantor::TextResult(content));
	    setStatus(Cantor::Expression::Done);
        }else if (type.name().contains(QLatin1String("image")))
        {
            setResult(new Cantor::ImageResult(QUrl::fromLocalFile(file)));
	    setStatus(Cantor::Expression::Done);
        }
        else
        {
            // File has unsupported mime type, but we suspect, that it is text, so will open the file in Cantor script editor
            // Even if it don't text, the script editor can deals with it.
            setResult(new Cantor::TextResult(QLatin1String("")));
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

