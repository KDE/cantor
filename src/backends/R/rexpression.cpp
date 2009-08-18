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


#include <kdebug.h>
#include <klocale.h>
#include <kmimetype.h>
#include <QFile>
#include <QTimer>
#include <QStringList>

RExpression::RExpression( MathematiK::Session* session ) : MathematiK::Expression(session)
{
    kDebug();

}

RExpression::~RExpression()
{

}


void RExpression::evaluate()
{
    kDebug()<<"evaluating "<<command();
    setStatus(MathematiK::Expression::Computing);
    if(command().startsWith('?'))
        m_isHelpRequest=true;
    else
        m_isHelpRequest=false;

    static_cast<RSession*>(session())->queueExpression(this);
}

void RExpression::interrupt()
{
    kDebug()<<"interrupting command";
    if(status()==MathematiK::Expression::Computing)
        session()->interrupt();
    setStatus(MathematiK::Expression::Interrupted);
}

void RExpression::finished(int returnCode, const QString& text)
{
    if(returnCode==RExpression::SuccessCode)
    {
        setResult(new MathematiK::TextResult(text));
        setStatus(MathematiK::Expression::Done);
    }else if (returnCode==RExpression::ErrorCode)
    {
        setResult(new MathematiK::TextResult(text));
        setStatus(MathematiK::Expression::Error);
        setErrorMessage(text);
    }
}

void RExpression::evaluationStarted()
{
    setStatus(MathematiK::Expression::Computing);
}

void RExpression::addInformation(const QString& information)
{
    static_cast<RSession*>(session())->sendInputToServer(information);
}

void RExpression::showFilesAsResult(const QStringList& files)
{
    kDebug()<<"showing files: "<<files;
    foreach(const QString& file, files)
    {
        KMimeType::Ptr type=KMimeType::findByUrl(file);
        kDebug()<<"MimeType: "<<type->name();
        if(type->is("application/postscript"))
        {
            kDebug()<<"its PostScript";
            setResult(new MathematiK::EpsResult(file));
        }else if(type->is("text/plain"))
        {
            kDebug()<<"its plain text";
            QFile f(file);
            if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                setResult(new MathematiK::TextResult(i18n("Error opening file %1", file)));
                setErrorMessage(i18n("Error opening file %1", file));
                setStatus(MathematiK::Expression::Error);
            }
            QString content=QTextStream(&f).readAll();
            //replace appearing backspaces, as they mess the whole output up
            content.remove(QRegExp(".\b"));
            //Replace < and > with their html code, so they won't be confused as html tags
            content.replace( '<' ,  "&lt;");
            content.replace( '>' ,  "&gt;");

            kDebug()<<"content: "<<content;
            if(m_isHelpRequest)
                setResult(new MathematiK::HelpResult(content));
            else
                setResult(new MathematiK::TextResult(content));
        }else if (type->name().contains("image"))
        {
            setResult(new MathematiK::ImageResult(file));
        }
        else
        {
            setResult(new MathematiK::TextResult(i18n("cannot open file %1: Unknown MimeType", file)));
            setErrorMessage(i18n("cannot open file %1: Unknown MimeType", file));
            setStatus(MathematiK::Expression::Error);
        }
    }
}

#include "rexpression.moc"
