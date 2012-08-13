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

#include "maximaexpression.h"

#include <config-cantorlib.h>

#include "maximasession.h"
#include "textresult.h"
#include "epsresult.h"
#include "imageresult.h"
#include "helpresult.h"
#include "settings.h"

#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>
#include <ktemporaryfile.h>
#include <QTimer>
#include <QRegExp>
#include <QChar>

MaximaExpression::MaximaExpression( Cantor::Session* session ) : Cantor::Expression(session)
{
    kDebug();
    m_tempFile=0;
}

MaximaExpression::~MaximaExpression()
{

}


void MaximaExpression::evaluate()
{
    kDebug()<<"evaluating "<<command();
    setStatus(Cantor::Expression::Computing);

    m_isHelpRequest=false;
    m_isPlot=false;
    if(m_tempFile)
        m_tempFile->deleteLater();
    m_tempFile=0;
    //check if this is a ?command
    if(command().startsWith('?')||command().startsWith(QLatin1String("describe("))||command().startsWith(QLatin1String("example(")))
        m_isHelpRequest=true;

    if(command().contains(QRegExp("(?:plot2d|plot3d)\\s*\\([^\\)]")) && MaximaSettings::self()->integratePlots() && !command().contains("psfile"))
    {
        m_isPlot=true;
        m_tempFile=new KTemporaryFile();
        m_tempFile->setPrefix( "cantor_maxima-" );
#ifdef WITH_EPS
        m_tempFile->setSuffix( ".eps" );
#else
        m_tempFile->setSuffix( ".png" );
#endif
        m_tempFile->open();

        disconnect(&m_fileWatch, SIGNAL(dirty(const QString&)), this, SLOT(imageChanged()));
        m_fileWatch.addFile(m_tempFile->fileName());
        connect(&m_fileWatch, SIGNAL(dirty(const QString&)), this, SLOT(imageChanged()));
    }

    //if the whole command consists of a comment, drop it
    static const QRegExp commentRegExp("^/\\*.*\\*/$");
    if(commentRegExp.exactMatch(command()))
       return;

    //also drop empty commands
    if(command().isEmpty())
    {
        kDebug()<<"dropping";
        return;
    }

    dynamic_cast<MaximaSession*>(session())->appendExpressionToQueue(this);

}

void MaximaExpression::interrupt()
{
    kDebug()<<"interrupting";
    dynamic_cast<MaximaSession*>(session())->interrupt(this);
    setStatus(Cantor::Expression::Interrupted);
}

QString MaximaExpression::internalCommand()
{
    QString cmd=command();

    if(m_isPlot)
    {
        if(!m_tempFile)
        {
            kDebug()<<"plotting without tempFile";
            return QString();
        }
        QString fileName = m_tempFile->fileName();

#ifdef WITH_EPS
        const QString psParam="[gnuplot_ps_term_command, \"set size 1.0,  1.0; set term postscript eps color solid \"]";
        const QString plotParameters = "[psfile, \""+ fileName+"\"],"+psParam;
#else
        const QString plotParameters = "[gnuplot_term, \"png size 500,340\"], [gnuplot_out_file, \""+fileName+"\"]";

#endif
        cmd.replace(QRegExp("((plot2d|plot3d)\\s*\\(.*)\\)([;\n]|$)"), "\\1, "+plotParameters+");");

    }

    if (!cmd.endsWith('$'))
    {
        if (!cmd.endsWith(QLatin1String(";")))
            cmd+=';';
    }

    //remove all newlines, as maxima isn't sensitive about
    //whitespaces, and without newlines the whole command
    //is executed at once, without outputting an input
    //prompt after each line
    cmd.remove('\n');

    //lisp-quiet doesn't print a prompt after the command
    //is completed, which causes the parsing to hang.
    //replace the command with the non-quiet version
    cmd.replace(QRegExp("^:lisp-quiet"), ":lisp");

    return cmd;
}

void MaximaExpression::forceDone()
{
    kDebug()<<"forcing Expression state to DONE";
    setResult(0);
    setStatus(Cantor::Expression::Done);
}

void MaximaExpression::addInformation(const QString& information)
{
    kDebug()<<"adding information";
    QString inf=information;
    if(!inf.endsWith(';'))
        inf+=';';
    Cantor::Expression::addInformation(inf);

    dynamic_cast<MaximaSession*>(session())->sendInputToProcess(inf+'\n');
}

inline void skipWhitespaces(int* idx, const QString& txt)
{
    for(;(txt[*idx]).isSpace();++(*idx));
}

QStringRef readXmlOpeningTag(int* idx, const QString& txt, bool* isComplete=0)
{
    kDebug()<<"trying to read an opening tag";
    skipWhitespaces(idx, txt);

    if(isComplete)
        *isComplete=false;

    if(txt[*idx]!='<')
    {
        kDebug()<<"THIS is NOT AN OPENING TAG, stupid! "<<endl
                <<"Dropping everything until next opening; This starts with a " <<txt[*idx];
        int newIdx=txt.indexOf('<', *idx);
        if(newIdx==-1) //no more opening brackets in this string
        {
            return QStringRef();
        }else
        {
            (*idx)=newIdx+1;
        }

    }else
    {
        ++ (*idx);
    }

    int startIndex=*idx;
    int length=0;
    QString name;
    while(*idx<txt.size())
    {
        const QChar c=txt[*idx];
        ++(*idx);

        if(c=='>')
        {
            if(isComplete)
                *isComplete=true;
            break;
        }else
            length++;
    }

    return QStringRef(&txt, startIndex, length);
}

QStringRef readXmlTagContent(int* idx, const QString& txt, const QStringRef& name, bool* isComplete=0)
{
    bool readingClosingTag=false;
    int contentStartIdx=*idx;
    unsigned int contentLength=0;
    int currentTagStartIdx=-1;
    unsigned int currentTagLength=0;

    if(isComplete)
        *isComplete=false;

    while(*idx<txt.size())
    {
        const QChar c=txt[*idx];

        if(c=='/'&&(*idx)>0&&txt[(*idx)-1]=='<')
        {
            //remove the opening <
            contentLength--;
            currentTagStartIdx=*idx+1;
            currentTagLength=0;
            readingClosingTag=true;
        }
        else if(readingClosingTag)
        {
            if(c=='>')
            {
                const QStringRef currentTagName(&txt, currentTagStartIdx, currentTagLength);
                kDebug()<<"a tag just closed: "<<currentTagName;

                if(currentTagName==name)
                {
                    //eat up the closing >
                    ++(*idx);
                    if(isComplete)
                        (*isComplete)=true;
                    break;
                }

                readingClosingTag=false;
            }else
                currentTagLength++;
        }
        else
            contentLength++;


        ++(*idx);

    }

    if(contentStartIdx+contentLength>txt.size())
    {
        kDebug()<<"something is wrong: "<<contentStartIdx+contentLength<<
            " vs: "<<txt.size();
    }
    return QStringRef(&txt,contentStartIdx, contentLength);
}

bool MaximaExpression::parseOutput(QString& out)
{
    enum ParserStatus{ReadingOpeningTag, ReadingClosingTag, ReadingText};
    int idx=0;
    ParserStatus status;
    QStringRef tagName;
    QStringRef errorBuffer;
    kDebug()<<"attempting to parse "<<out;

    QChar c;
    //first read the part not enclosed in tags. it most likely belongs to an error message
    int idx1=out.indexOf("<prompt>");
    int idx2=out.indexOf("<result>");

    idx1=(idx1==-1) ? out.size():idx1;
    idx2=(idx2==-1) ? out.size():idx2;
    idx=qMin(idx1, idx2);

    kDebug()<<"out.size(): "<<out.size();
    kDebug()<<"idx1: "<<idx1;
    kDebug()<<"idx2: "<<idx2;


    errorBuffer=QStringRef(&out, 0, idx);
    kDebug()<<"the unmatched part of the output is: "<<errorBuffer;

    int numResults=0;
    QString textBuffer;
    QString latexBuffer;

    Cantor::Result* result=0;
    while(idx<out.size())
    {
        skipWhitespaces(&idx, out);

        const QStringRef& tag=readXmlOpeningTag(&idx, out);

        kDebug()<<"big loop read " <<tag;
        if(tag=="result")
        {
            kDebug()<<"hey, I got a result";

            if(numResults>0)
            {
                textBuffer.append("\n");
                latexBuffer.append("\n");
            }

            result=parseResult(&idx, out, textBuffer, latexBuffer);
            numResults++;

            kDebug()<<"got "<<numResults<<"th result.";

        }else if (tag=="prompt")
        {
            kDebug()<<"i got a prompt: "<<idx;

            skipWhitespaces(&idx, out);

            //We got a child tag
            if(out[idx]=='<')
            {
                const QStringRef& childTag=readXmlOpeningTag(&idx, out);
                kDebug()<<"got an information request!"<<childTag;

                QStringRef text;
                QStringRef latex;
                while(idx<out.size())
                {
                    const QStringRef& type=readXmlOpeningTag(&idx, out);
                    kDebug()<<"its a "<<type;
                    if(type=="/result")
                        break;
                    const QStringRef& content=readXmlTagContent(&idx, out, type);

                    if(type=="text")
                        text=content;
                    else if(type=="latex")
                        latex=content;

                    kDebug()<<"content: "<<content;
                }

                bool isComplete;
                //readup the rest of the element and discard it
                readXmlTagContent(&idx, out,tag, &isComplete);

                if(!isComplete)
                    return false;

                //send out the information request
                emit needsAdditionalInformation(text.toString());

                out=out.mid(idx);
                idx=0;

                return true;

            }else //got a regular prompt. Just read it all
            {
                bool isComplete;
                const QStringRef& content=readXmlTagContent(&idx, out, tag, &isComplete);

                if(!isComplete)
                    return false;

                QString errorMsg=errorBuffer.toString();
                if(!errorMsg.trimmed().isEmpty())
                {
                    //Replace < and > with their html code, so they won't be confused as html tags
                    errorMsg.replace( '<' , "&lt;");
                    errorMsg.replace( '>' , "&gt;");

                    if(command().startsWith(":lisp")||command().startsWith(":lisp-quiet"))
                    {
                        Cantor::TextResult* result=new Cantor::TextResult(errorMsg);
                        setResult(result);
                        setStatus(Cantor::Expression::Done);
                    }else
                    if(m_isHelpRequest) //Help Messages are also provided in the errorBuffer.

                    {
                        Cantor::HelpResult* result=new Cantor::HelpResult(errorMsg);
                        setResult(result);

                        setStatus(Cantor::Expression::Done);
                    }else
                    {
                        setErrorMessage(errorMsg.trimmed());
                        setStatus(Cantor::Expression::Error);
                    }
                }
                else
                {
                    if(errorMessage().isEmpty())
                    {
                        setResult(result);
                        setStatus(Cantor::Expression::Done);
                    }
                    else
                        setStatus(Cantor::Expression::Error);
                }

                out=out.mid(idx);
                idx=0;

                return true;
            }
        }else
        {
            kDebug()<<"tag: "<<tag;
            kDebug()<<"WTF are you doing?";
        }
    }

    //show partial result
    setResult(result);
    return false;
}

Cantor::Result* MaximaExpression::parseResult(int* idx, QString& out,
                                              QString& textBuffer, QString& latexBuffer)
{
    bool isLatexComplete=false;
    QString latex;
    QString text;

    while(*idx<out.size())
    {
        bool isComplete;
        const QStringRef& type=readXmlOpeningTag(idx, out);
        kDebug()<<"its a "<<type;
        if(type=="/result")
            break;

        const QStringRef& content=readXmlTagContent(idx, out, type, &isComplete);

        if(type=="text")
            text=content.toString().trimmed();
        else if(type=="latex")
        {
            isLatexComplete=isComplete;
            latex=content.toString().trimmed();
        }

        kDebug()<<"content: "<<content;
    }

    //Replace < and > with their html code, so they won't be confused as html tags
    text.replace( '<' , "&lt;");
    text.replace( '>' , "&gt;");

    QRegExp outputPromptRegexp=QRegExp('^'+MaximaSession::MaximaOutputPrompt.pattern());
    int idxOfPrompt=outputPromptRegexp.indexIn(text);
    text.remove(idxOfPrompt, outputPromptRegexp.matchedLength());

    //find the number if this output in the MaximaOutputPrompt
    QString prompt=outputPromptRegexp.cap(0).trimmed();
    QString id=prompt.mid(3, prompt.length()-4);
    setId(id.toInt());
    kDebug()<<"prompt: "<<prompt<<" id: "<<id;



    if(m_tempFile)
    {
        QTimer::singleShot(500, this, SLOT(imageChanged()));
    }

    Cantor::TextResult* result=0;

    //if this is not the first result, prepend the results
    //found in the earlier tags.
    textBuffer.append(text);

    //if the <latex> element wasn't read completely, there
    //is no point in trying to render it. Use text for
    //incomplete results.
    if(!isLatexComplete
       ||(latexBuffer.trimmed().isEmpty()&&latex.isEmpty())
       ||m_isHelpRequest)
    {
        kDebug()<<"using text";
        result=new Cantor::TextResult(textBuffer);
    }else
    {
        kDebug()<<"using latex";
        //strip away the latex code for the label.
        //it is contained in an \mbox{} call
        int i;
        int pcount=0;
        for(i=latex.indexOf("\\mbox{")+5;i<latex.size();i++)
        {
            if(latex[i]=='{')
                pcount++;
            else if(latex[i]=='}')
                pcount--;

            if(pcount==0)
                break;
        }

        kDebug()<<"stripping i="<<i<<" characters";
        latex=latex.mid(i+1);

        //no need to render empty latex.
        if(latexBuffer.trimmed().isEmpty()&&latex.trimmed().isEmpty())
        {
            if(m_isPlot)
                result=new Cantor::TextResult(i18n("Waiting for Image..."));
            else
                result=0;
        }else
        {
            latex.prepend("\\begin{eqnarray*}\n");
            latex.append("\n\\end{eqnarray*}");
            latexBuffer.append(latex);
            result=new Cantor::TextResult(latexBuffer);
            result->setFormat(Cantor::TextResult::LatexFormat);
        }
    }


    return result;
}

void MaximaExpression::imageChanged()
{
    kDebug()<<"the temp image has changed";
    if(m_tempFile->size()>0)
    {
#ifdef WITH_EPS
        setResult( new Cantor::EpsResult( KUrl(m_tempFile->fileName()) ) );
#else
        setResult( new Cantor::ImageResult( KUrl(m_tempFile->fileName()) ) );
#endif
        setStatus(Cantor::Expression::Done);
    }
}


QString MaximaExpression::additionalLatexHeaders()
{
    return QString::null;
}

#include "maximaexpression.moc"
