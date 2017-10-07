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
#include <QRegExp>

RExpression::RExpression( Cantor::Session* session ) : Cantor::Expression(session)
{
    qDebug();

}

RExpression::~RExpression()
{

}


void RExpression::evaluate()
{
    qDebug()<<"evaluating "<<command();
    setStatus(Cantor::Expression::Computing);
    if(command().trimmed().startsWith(QLatin1Char('?'))  || command().trimmed().startsWith(QLatin1String("help")))
        m_isHelpRequest=true;
    else
        m_isHelpRequest=false;


    RSession* session = dynamic_cast<RSession*>(this->session());

    /* check if the command is a quit command. following are quit commands supported by R
     * quit(save = "default", status = 0, runLast = TRUE)
     * q(save = "default", status = 0, runLast = TRUE)
    */
    QRegExp regexp;
    regexp.setCaseSensitivity(Qt::CaseSensitive);
    regexp.setPattern(QLatin1String("\\s*q\\s*\\([\\w\\W]*\\)\\s*$|\\s*quit\\s*\\([\\w\\W]*\\)\\s*$"));
    if(regexp.exactMatch(command().trimmed())) {
        // since it's a quit command, logout from the session
        setStatus(Cantor::Expression::Done);
        session->logout();
        return;
    }

    // done evaluation, finally running the command
    session->runExpression(this);


}

void RExpression::parseOutput(QString output)
{
    // remove garabage from output

    output.replace(command(), QLatin1String(""));
    output.replace(QLatin1String(">"), QLatin1String(""));
    output = output.trimmed();

    qDebug() << " final output of the command " << command() << output << endl;

    if(m_isHelpRequest)
        setResult(new Cantor::HelpResult(output));
    else
        setResult(new Cantor::TextResult(output));

    setStatus(Cantor::Expression::Done);
}

void RExpression::parseError(QString error)
{
    error.replace(command(), QLatin1String(""));
    error.replace(QLatin1String(">"), QLatin1String(""));
    error = error.trimmed();

    setResult(new Cantor::TextResult(error));

    setStatus(Cantor::Expression::Error);
    setErrorMessage(error);
}

void RExpression::interrupt()
{
    qDebug()<<"interrupting command";
    if(status()==Cantor::Expression::Computing)
        session()->interrupt();
    setStatus(Cantor::Expression::Interrupted);
}

void RExpression::addInformation(const QString& information)
{
    /* not using this anymore since it was making use of rserver
     *
    */
//    static_cast<RSession*>(session())->sendInputToServer(information);
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
        } else if(type.inherits(QLatin1String("text/plain")))
        {
            //Htmls are also plain texts, combining this in one
            if(type.inherits(QLatin1String("text/html")))
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
            if (!type.inherits(QLatin1String("text/html")))
            {
                //Escape whitespace
                content.replace( QLatin1Char(' '), QLatin1String("&nbsp;"));
                //replace appearing backspaces, as they mess the whole output up
                content.remove(QRegExp(QLatin1String(".\b")));
                //Replace < and > with their html code, so they won't be confused as html tags
                content.replace( QLatin1Char('<') ,  QLatin1String("&lt;"));
                content.replace( QLatin1Char('>') ,  QLatin1String("&gt;"));
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
            setResult(new Cantor::TextResult(i18n("cannot open file %1: Unknown MimeType", file)));
            setErrorMessage(i18n("cannot open file %1: Unknown MimeType", file));
            setStatus(Cantor::Expression::Error);
        }
    }
}

#include "rexpression.moc"
