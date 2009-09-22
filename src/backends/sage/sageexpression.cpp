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

#include "sageexpression.h"

#include "sagesession.h"
#include "textresult.h"
#include "imageresult.h"
#include "helpresult.h"

#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>
#include <QRegExp>

SageExpression::SageExpression( MathematiK::Session* session ) : MathematiK::Expression(session)
{
    kDebug();
}

SageExpression::~SageExpression()
{

}

void SageExpression::evaluate()
{
    kDebug()<<"evaluating "<<command();
    setStatus(MathematiK::Expression::Computing);
    m_imagePath.clear();

    m_isHelpRequest=false;

    //check if this is a ?command
    if(command().startsWith('?')||command().endsWith('?'))
        m_isHelpRequest=true;

    //coun't how many newlines are in the command,
    //as sage will output one "sage: " or "....:" for
    //each.
    m_promptCount=command().count('\n')+1;

    dynamic_cast<SageSession*>(session())->appendExpressionToQueue(this);
}

void SageExpression::interrupt()
{
    kDebug()<<"interrupting";
    dynamic_cast<SageSession*>(session())->sendSignalToProcess(2);
}

void SageExpression::parseOutput(const QString& text)
{
    QString output=text;
    //replace appearing backspaces, as they mess the whole output up
    output.remove(QRegExp(".\b"));

    m_outputCache+=output;

    const QString promptRegexpBase("(^|\\n)%1($|\\n)");
    const QRegExp promptRegexp(promptRegexpBase.arg(QRegExp::escape(SageSession::SagePrompt)));
    const QRegExp altPromptRegexp(promptRegexpBase.arg(QRegExp::escape(SageSession::SageAlternativePrompt)));

    m_promptCount-=output.count(promptRegexp);
    m_promptCount-=output.count(altPromptRegexp);

    if(m_promptCount<=0)
    {
        if(m_promptCount<0)
            kError()<<"got too many prompts";

        //if the output ends with an AlternativePrompt, this means that
        //Sage is expecting additional input, allthough m_promptCount==0
        //indicates that all information has been passed to sage.
        //This means that the user has entered an invalid command.
        //interrupt it and show an error message
        if(m_outputCache.endsWith(SageSession::SageAlternativePrompt))
        {
            interrupt();
            setErrorMessage(i18n("Syntax Error"));
            setStatus(MathematiK::Expression::Error);
            return;
        }

        m_outputCache.remove(SageSession::SagePrompt);
        m_outputCache.remove(SageSession::SageAlternativePrompt);
        m_outputCache=m_outputCache.trimmed();
        evalFinished();
    }

}

void SageExpression::parseError(const QString& text)
{
    kDebug()<<"error";
    setResult(new MathematiK::TextResult(text));
    setStatus(MathematiK::Expression::Error);
}

void SageExpression::addFileResult( const QString& path )
{
  KUrl url( path );
  if ( url.fileName().endsWith(QLatin1String(".png")) )
  {
    kDebug()<<"adding file "<<path<<"   "<<url;
    m_imagePath=path;
  }
}

void SageExpression::evalFinished()
{
    kDebug()<<"evaluation finished";
    kDebug()<<m_outputCache;

    if ( m_imagePath.isNull() ) //If this result contains a file, drop the text information
    {
        MathematiK::TextResult* result=0;

        QString stripped=m_outputCache;
        const bool isHtml=stripped.startsWith(QLatin1String("<html>"));
        const bool isLatex=m_outputCache.contains("class=\"math\""); //Check if it's latex stuff
        if(isLatex) //It's latex stuff so encapsulate it into an eqnarray environment
        {
            stripped.replace("<html>", "\\begin{eqnarray*}");
            stripped.replace("</html>", "\\end{eqnarray*}");
        }

        //strip html tags
        if(isHtml)
        {
            stripped.remove( QRegExp( "<[a-zA-Z\\/][^>]*>" ) );
        }
        else
        {
            //Replace < and > with their html code, so they won't be confused as html tags
            stripped.replace( '<' , "&lt;");
            stripped.replace( '>' , "&gt;");
        }
        if (stripped.endsWith('\n'))
            stripped.chop(1);

        if (m_isHelpRequest)
        {
            //make things quoted in `` `` bold
            stripped.replace(QRegExp("``([^`]*)``"), "<b>\\1</b>");

            result=new MathematiK::HelpResult(stripped);
        }
        else
        {
            result=new MathematiK::TextResult(stripped);
        }

        if(isLatex)
            result->setFormat(MathematiK::TextResult::LatexFormat);

        setResult(result);
    }
    else
    {
      setResult( new MathematiK::ImageResult( KUrl(m_imagePath ),i18n("Result of %1" , command() ) ) );
    }
    setStatus(MathematiK::Expression::Done);
}

void SageExpression::onProcessError(const QString& msg)
{
    QString errMsg=i18n("%1\nThe last output was: \n %2", msg, m_outputCache.trimmed());
    setErrorMessage(errMsg);
    setStatus(MathematiK::Expression::Error);
}

QString SageExpression::additionalLatexHeaders()
{
    //The LaTeX sage provides needs the amsmath package. So include it in the header
    return "\\usepackage{amsmath}\n";
}

#include "sageexpression.moc"
