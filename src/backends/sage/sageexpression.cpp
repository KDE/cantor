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

SageExpression::SageExpression( Cantor::Session* session ) : Cantor::Expression(session)
{
    kDebug();
}

SageExpression::~SageExpression()
{

}

void SageExpression::evaluate()
{
    kDebug()<<"evaluating "<<command();
    setStatus(Cantor::Expression::Computing);
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
    dynamic_cast<SageSession*>(session())->waitForNextPrompt();

    setStatus(Cantor::Expression::Interrupted);
}

void SageExpression::parseOutput(const QString& text)
{
    QString output=text;
    //remove carriage returns, we only use \n internally
    output.remove("\r");
    //replace appearing backspaces, as they mess the whole output up
    output.remove(QRegExp(".\b"));
    //replace Escape sequences (only tested with `ls` command)
    output.remove(QRegExp("\e\\][^\a]*\a"));

    const QString promptRegexpBase("(^|\\n)%1");
    const QRegExp promptRegexp(promptRegexpBase.arg(QRegExp::escape(SageSession::SagePrompt)));
    const QRegExp altPromptRegexp(promptRegexpBase.arg(QRegExp::escape(SageSession::SageAlternativePrompt)));

    bool endsWithAlternativePrompt=output.endsWith(SageSession::SageAlternativePrompt);

    //remove all prompts. we do this in a loop, because after we removed the first prompt,
    //there could be a second one, that isn't matched by promptRegexp in the first run, because
    //it originally isn't at the beginning of a line.
    int index=-1, index2=-1;
    while ( (index=output.indexOf(promptRegexp)) != -1 || (index2=output.indexOf(altPromptRegexp)) != -1 )
    {
        kDebug()<<"got prompt"<<index<<"  "<<index2;
        if(index!=-1)
        {
            m_promptCount--;

            //remove this prompt, the if is needed, because, if the prompt is on the
            //beginning of the string, index points to the "s", if it is within the string
            //index points to the newline
            if(output[index]=='\n')
                output.remove(index+1, SageSession::SagePrompt.length());
            else
                output.remove(index, SageSession::SagePrompt.length());
        }

        if(index2!=-1)
        {
            m_promptCount--;

            //see comment above, for the reason for this "if"
            if(output[index2]=='\n')
                output.remove(index2+1, SageSession::SageAlternativePrompt.length());
            else
                output.remove(index2, SageSession::SageAlternativePrompt.length());
        }

        //reset the indices
        index=index2=-1;
    }

    m_outputCache+=output;

    if(m_promptCount<=0)
    {
        if(m_promptCount<0)
            kError()<<"got too many prompts";

        //if the output ends with an AlternativePrompt, this means that
        //Sage is expecting additional input, allthough m_promptCount==0
        //indicates that all information has been passed to sage.
        //This means that the user has entered an invalid command.
        //interrupt it and show an error message
        if(endsWithAlternativePrompt)
        {
            interrupt();
            setErrorMessage(i18n("Syntax Error"));
            setStatus(Cantor::Expression::Error);
            return;
        }

        m_outputCache=m_outputCache.trimmed();
        evalFinished();
    }

}

void SageExpression::parseError(const QString& text)
{
    kDebug()<<"error";
    setResult(new Cantor::TextResult(text));
    setStatus(Cantor::Expression::Error);
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
        Cantor::TextResult* result=0;

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

            result=new Cantor::HelpResult(stripped);
        }
        else
        {
            result=new Cantor::TextResult(stripped);
        }

        if(isLatex)
            result->setFormat(Cantor::TextResult::LatexFormat);

        setResult(result);
    }
    else
    {
      setResult( new Cantor::ImageResult( KUrl(m_imagePath ),i18n("Result of %1" , command() ) ) );
    }
    setStatus(Cantor::Expression::Done);
}

void SageExpression::onProcessError(const QString& msg)
{
    QString errMsg=i18n("%1\nThe last output was: \n %2", msg, m_outputCache.trimmed());
    setErrorMessage(errMsg);
    setStatus(Cantor::Expression::Error);
}

QString SageExpression::additionalLatexHeaders()
{
    //The LaTeX sage provides needs the amsmath package. So include it in the header
    return "\\usepackage{amsmath}\n";
}

#include "sageexpression.moc"
