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
#include "animationresult.h"
#include "helpresult.h"

#include <QDebug>
#include <KLocale>
#include <KUrl>
#include <KMimeType>
#include <QRegExp>

SageExpression::SageExpression( Cantor::Session* session ) : Cantor::Expression(session)
{
    qDebug();
}

SageExpression::~SageExpression()
{

}

void SageExpression::evaluate()
{
    qDebug()<<"evaluating "<<command();
    setStatus(Cantor::Expression::Computing);
    m_imagePath.clear();

    m_isHelpRequest=false;

    //check if this is a ?command
    if(command().startsWith(QLatin1Char('?'))||command().endsWith(QLatin1Char('?')))
        m_isHelpRequest=true;

    //coun't how many newlines are in the command,
    //as sage will output one "sage: " or "....:" for
    //each.
    m_promptCount=command().count(QLatin1Char('\n'))+2;

    dynamic_cast<SageSession*>(session())->appendExpressionToQueue(this);
}

void SageExpression::interrupt()
{
    qDebug()<<"interrupting";
    dynamic_cast<SageSession*>(session())->sendSignalToProcess(2);
    dynamic_cast<SageSession*>(session())->waitForNextPrompt();

    setStatus(Cantor::Expression::Interrupted);
}

void SageExpression::parseOutput(const QString& text)
{
    QString output=text;
    //remove carriage returns, we only use \n internally
    output.remove(QLatin1Char('\r'));
    //replace appearing backspaces, as they mess the whole output up
    output.remove(QRegExp(QLatin1String(".\b")));
    //replace Escape sequences (only tested with `ls` command)
    const QChar ESC(0x1b);
    output.remove(QRegExp(QString(ESC)+QLatin1String("\\][^\a]*\a")));

    const QString promptRegexpBase(QLatin1String("(^|\\n)%1"));
    const QRegExp promptRegexp(promptRegexpBase.arg(QRegExp::escape(QLatin1String(SageSession::SagePrompt))));
    const QRegExp altPromptRegexp(promptRegexpBase.arg(QRegExp::escape(QLatin1String(SageSession::SageAlternativePrompt))));

    bool endsWithAlternativePrompt=output.endsWith(QLatin1String(SageSession::SageAlternativePrompt));

    //remove all prompts. we do this in a loop, because after we removed the first prompt,
    //there could be a second one, that isn't matched by promptRegexp in the first run, because
    //it originally isn't at the beginning of a line.
    int index=-1, index2=-1;
    while ( (index=output.indexOf(promptRegexp)) != -1 || (index2=output.indexOf(altPromptRegexp)) != -1 )
    {
        qDebug()<<"got prompt"<<index<<"  "<<index2;
        if(index!=-1)
        {
            m_promptCount--;

            //remove this prompt, the if is needed, because, if the prompt is on the
            //beginning of the string, index points to the "s", if it is within the string
            //index points to the newline
            if(output[index]==QLatin1Char('\n'))
                output.remove(index+1, SageSession::SagePrompt.length());
            else
                output.remove(index, SageSession::SagePrompt.length());
        }

        if(index2!=-1)
        {
            m_promptCount--;

            //see comment above, for the reason for this "if"
            if(output[index2]==QLatin1Char('\n'))
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
            qDebug()<<"got too many prompts";

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
    qDebug()<<"error";
    setResult(new Cantor::TextResult(text));
    setStatus(Cantor::Expression::Error);
}

void SageExpression::addFileResult( const QString& path )
{
  KUrl url( path );
  KMimeType::Ptr type=KMimeType::findByUrl(url);
  if(m_imagePath.isEmpty()||type->name().contains(QLatin1String("image"))||path.endsWith(QLatin1String(".png"))||path.endsWith(QLatin1String(".gif")))
  {
      m_imagePath=path;
  }
}

void SageExpression::evalFinished()
{
    qDebug()<<"evaluation finished";
    qDebug()<<m_outputCache;

    //check if our image path contains a valid image that we can try to show
    bool hasImage=!m_imagePath.isNull();

    if ( !hasImage ) //If this result contains a file, drop the text information
    {
        Cantor::TextResult* result=0;

        QString stripped=m_outputCache;
        const bool isHtml=stripped.contains(QLatin1String("<html>"));
        const bool isLatex=m_outputCache.contains(QLatin1String("class=\"math\""))||m_outputCache.contains(QRegExp(QLatin1String("type=\"math/tex[^\"]*\""))); //Check if it's latex stuff
        if(isLatex) //It's latex stuff so encapsulate it into an eqnarray environment
        {
            stripped.replace(QLatin1String("<html>"), QLatin1String("\\begin{eqnarray*}"));
            stripped.replace(QLatin1String("</html>"), QLatin1String("\\end{eqnarray*}"));
        }

        //strip html tags
        if(isHtml)
        {
            stripped.remove( QRegExp( QLatin1String("<[a-zA-Z\\/][^>]*>") ) );
        }
        else
        {
            //Replace < and > with their html code, so they won't be confused as html tags
            stripped.replace( QLatin1Char('<') , QLatin1String("&lt;"));
            stripped.replace( QLatin1Char('>') , QLatin1String("&gt;"));
        }
        if (stripped.endsWith(QLatin1Char('\n')))
            stripped.chop(1);

        if (m_isHelpRequest)
        {
            //Escape whitespace
            stripped.replace( QLatin1Char(' '), QLatin1String("&nbsp;"));

            //make things quoted in `` `` bold
            stripped.replace(QRegExp(QLatin1String("``([^`]*)``")), QLatin1String("<b>\\1</b>"));

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
	KMimeType::Ptr type=KMimeType::findByUrl(KUrl(m_imagePath));
        if(type->is(QLatin1String("image/gif")))
            setResult( new Cantor::AnimationResult( KUrl(m_imagePath ),i18n("Result of %1" , command() ) ) );
        else
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
    return QLatin1String("\\usepackage{amsmath}\n");
}

#include "sageexpression.moc"
