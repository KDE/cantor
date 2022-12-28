/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "sageexpression.h"

#include "sagesession.h"
#include "textresult.h"
#include "imageresult.h"
#include "animationresult.h"
#include "helpresult.h"

#include <QDebug>
#include <KLocalizedString>
#include <QMimeDatabase>
#include <QRegularExpression>

SageExpression::SageExpression( Cantor::Session* session, bool internal ) : Cantor::Expression(session, internal),
    m_isHelpRequest(false),
    m_promptCount(0),
    m_syntaxError(false)
{
}

void SageExpression::evaluate()
{
    m_imagePath.clear();

    m_isHelpRequest=false;

    //check if this is a ?command or help command
    if( command().startsWith(QLatin1Char('?'))
        || command().endsWith(QLatin1Char('?'))
        || command().contains(QLatin1String("help("))
    )
        m_isHelpRequest=true;

    //coun't how many newlines are in the command,
    //as sage will output one "sage: " or "....:" for
    //each.
    m_promptCount=command().count(QLatin1Char('\n'))+2;

    session()->enqueueExpression(this);
}

void SageExpression::parseOutput(const QString& text)
{
    if (m_syntaxError)
    {
        setErrorMessage(i18n("Syntax Error"));
        setStatus(Cantor::Expression::Error);
        return;
    }

    QString output=text;
    //remove carriage returns, we only use \n internally
    output.remove(QLatin1Char('\r'));
    //replace appearing backspaces, as they mess the whole output up
    //with QRegularExpression/PCRE to make \b match a backspace put it inside []
    //see https://perldoc.perl.org/perlrecharclass.html#Bracketed-Character-Classes
    output.remove(QRegularExpression(QStringLiteral(".[\b]")));
    //replace Escape sequences (only tested with `ls` command)
    const QChar ESC(0x1b);
    output.remove(QRegularExpression(QString(ESC)+QLatin1String("\\][^\a]*\a")));

    const QString promptRegexpBase(QLatin1String("(^|\\n)%1"));
    const QRegularExpression promptRegexp(promptRegexpBase.arg(
                QRegularExpression::escape(QLatin1String(SageSession::SagePrompt))));
    const QRegularExpression altPromptRegexp(promptRegexpBase.arg(
                QRegularExpression::escape(QLatin1String(SageSession::SageAlternativePrompt))));

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
        //Sage is expecting additional input, although m_promptCount==0
        //indicates that all information has been passed to sage.
        //This means that the user has entered an invalid command.
        //interrupt it and show an error message
        if(endsWithAlternativePrompt)
        {
            // Exit from sage additional input mode
            static_cast<SageSession*>(session())->sendInputToProcess(QLatin1String("\x03"));
            m_syntaxError = true;
        }
        else
        {
            evalFinished();
        }
    }

}

void SageExpression::parseError(const QString& text)
{
    qDebug()<<"error";
    setErrorMessage(text);
    setStatus(Cantor::Expression::Error);
}

void SageExpression::addFileResult( const QString& path )
{
  QUrl url = QUrl::fromLocalFile(path);
  QMimeDatabase db;
  QMimeType type = db.mimeTypeForUrl(url);

  if(m_imagePath.isEmpty()||type.name().contains(QLatin1String("image"))||path.endsWith(QLatin1String(".png"))||path.endsWith(QLatin1String(".gif")))
  {
      m_imagePath=path;
  }
}

void SageExpression::evalFinished()
{
    qDebug()<<"evaluation finished";
    qDebug()<<m_outputCache;

    //check if our image path contains a valid image that we can try to show
    const bool hasImage=!m_imagePath.isNull();

    if (!m_outputCache.isEmpty())
    {
        QString stripped=m_outputCache;
        const bool isHtml=stripped.contains(QLatin1String("<html>"));
        const bool isLatex=m_outputCache.contains(QLatin1String("\\newcommand{\\Bold}")); //Check if it's latex stuff
        if(isLatex) //It's latex stuff so encapsulate it into an eqnarray environment
        {
            int bol_command_len = QLatin1String("\\newcommand{\\Bold}[1]{\\mathbf{#1}}").size();
            int curr_index = stripped.indexOf(QLatin1String("\\newcommand{\\Bold}[1]{\\mathbf{#1}}"))+bol_command_len;
            // Add an & for the align environment
            stripped.insert(curr_index, QLatin1String("&"));
            // Strip away any additional "\\newcommand;{\\Bold}" so that it's compilable by LaTeX
            if(stripped.count(QLatin1String("\\newcommand{\\Bold}")) > 1){
                while(curr_index != -1){
                    curr_index = stripped.indexOf(QLatin1String("\\newcommand{\\Bold}[1]{\\mathbf{#1}}"), curr_index);
                    stripped.remove(curr_index, bol_command_len);
                    // Also add an & for left alignment
                    stripped.insert(curr_index, QLatin1String("&"));
                }
            }
            // Replace new-line characters with \\ for LaTeX's newline intepretation
            stripped.replace(QLatin1Char('\n'), QLatin1String("\\\\"));
            stripped.prepend(QLatin1String("\\begin{align*}"));
            stripped.append(QLatin1String("\\end{align*}"));
            // TODO: Remove for final merge
            qDebug()<<"NewCommand";
            qDebug()<<stripped;
        }

        //strip html tags
        if(isHtml)
        {
            stripped.remove( QRegularExpression( QStringLiteral("<[a-zA-Z\\/][^>]*>") ) );
        }

        if (stripped.endsWith(QLatin1Char('\n')))
            stripped.chop(1);

        if (m_isHelpRequest)
        {
            stripped = stripped.toHtmlEscaped();
            stripped.replace(QLatin1Char(' '), QLatin1String("&nbsp;"));
            stripped.replace(QLatin1Char('\n'), QLatin1String("<br/>\n"));

            //make things quoted in `` `` bold
            stripped.replace(QRegularExpression(QStringLiteral("``([^`]*)``")), QStringLiteral("<b>\\1</b>"));

            addResult(new Cantor::HelpResult(stripped, true));
        }
        else
        {
            Cantor::TextResult* result=new Cantor::TextResult(stripped);
            if(isLatex)
                result->setFormat(Cantor::TextResult::LatexFormat);
            addResult(result);
        }
    }

    if (hasImage)
    {
    QMimeDatabase db;
    QMimeType type = db.mimeTypeForUrl(QUrl::fromLocalFile(m_imagePath));
        if(type.inherits(QLatin1String("image/gif")))
            addResult( new Cantor::AnimationResult(QUrl::fromLocalFile(m_imagePath ),i18n("Result of %1" , command() ) ) );
        else
            addResult( new Cantor::ImageResult( QUrl::fromLocalFile(m_imagePath ),i18n("Result of %1" , command() ) ) );
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
    //The LaTeX sage needs the amsmath package and some specific macros.
    //So include them in the header.
    //More about the macros requirement in bug #312738
    return QLatin1String("\\usepackage{amsmath}\n"                   \
                         "\\newcommand{\\ZZ}{\\Bold{Z}}\n"           \
                         "\\newcommand{\\NN}{\\Bold{N}}\n"           \
                         "\\newcommand{\\RR}{\\Bold{R}}\n"           \
                         "\\newcommand{\\CC}{\\Bold{C}}\n"           \
                         "\\newcommand{\\QQ}{\\Bold{Q}}\n"           \
                         "\\newcommand{\\QQbar}{\\overline{\\QQ}}\n" \
                         "\\newcommand{\\GF}[1]{\\Bold{F}_{#1}}\n"   \
                         "\\newcommand{\\Zp}[1]{\\ZZ_{#1}}\n"        \
                         "\\newcommand{\\Qp}[1]{\\QQ_{#1}}\n"        \
                         "\\newcommand{\\Zmod}[1]{\\ZZ/#1\\ZZ}\n"    \
                         "\\newcommand{\\CDF}{\\Bold{C}}\n"          \
                         "\\newcommand{\\CIF}{\\Bold{C}}\n"          \
                         "\\newcommand{\\CLF}{\\Bold{C}}\n"          \
                         "\\newcommand{\\RDF}{\\Bold{R}}\n"          \
                         "\\newcommand{\\RIF}{\\Bold{I} \\Bold{R}}\n"\
                         "\\newcommand{\\RLF}{\\Bold{R}}\n"          \
                         "\\newcommand{\\CFF}{\\Bold{CFF}}\n");
}

