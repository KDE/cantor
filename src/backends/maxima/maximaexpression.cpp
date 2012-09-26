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
    Copyright (C) 2009-2012 Alexander Rieder <alexanderrieder@gmail.com>
 */

#include "maximaexpression.h"

#include <config-cantorlib.h>

#include "maximasession.h"
#include "textresult.h"
#include "epsresult.h"
#include "imageresult.h"
#include "helpresult.h"
#include "latexresult.h"
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

    //until we get the real output Id from maxima, set it to invalid
    setId(-1);

    m_isHelpRequest=false;
    m_isPlot=false;
    m_gotErrorContent=false;
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

    const QString& cmd=command();

    bool isComment = true;
    int commentLevel = 0;
    bool inString = false;
    for (int i = 0; i < cmd.size(); ++i) {
        if (cmd[i] == '\\') {
            ++i; // skip the next character
            if (commentLevel == 0 && !inString) {
                isComment = false;
            }
        } else if (cmd[i] == '"' && commentLevel == 0) {
            inString = !inString;
            isComment = false;
        } else if (cmd.mid(i,2) == "/*" && !inString) {
            ++commentLevel;
            ++i;
        } else if (cmd.mid(i,2) == "*/" && !inString) {
            if (commentLevel == 0) {
                kDebug() << "Comments mismatched!";
                setErrorMessage(i18n("Error: Too many */"));
                setStatus(Cantor::Expression::Error);
                return;
            }
            ++i;
            --commentLevel;
        } else if (isComment && commentLevel == 0 && !cmd[i].isSpace()) {
            isComment = false;
        }
    }

    if (commentLevel > 0) {
        kDebug() << "Comments mismatched!";
        setErrorMessage(i18n("Error: Too many /*"));
        setStatus(Cantor::Expression::Error);
        return;
    }
    if (inString) {
        kDebug() << "String not closed";
        setErrorMessage(i18n("Error: expected \" before ;"));
        setStatus(Cantor::Expression::Error);
        return;
    }
    if(isComment)
    {
        setStatus(Cantor::Expression::Done);
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

    //replace all newlines with spaces, as maxima isn't sensitive about
    //whitespaces, and without newlines the whole command
    //is executed at once, without outputting an input
    //prompt after each line
    cmd.replace('\n', ' ');

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


//The maxima backend is modified, so that it outputs
//xml-style tags around outputs, input prompts etc.
//the following are some simple helper functions to faciliate parsing
inline void skipWhitespaces(int* idx, const QString& txt)
{
    for(;*idx < txt.size() && (txt[*idx]).isSpace();++(*idx));
}

QStringRef readXmlOpeningTag(int* idx, const QString& txt, bool* isComplete=0)
{
    kDebug()<<"trying to read an opening tag";

    if (*idx >= txt.size())
        return QStringRef();

    skipWhitespaces(idx, txt);

    if(isComplete)
        *isComplete=false;

    if(txt[*idx]!='<')
    {
        kDebug()<<"This is NOT AN OPENING TAG."<<endl
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
    int contentLength=0;
    int currentTagStartIdx=-1;
    int currentTagLength=0;

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
        kDebug()<<"something is wrong with the content-length "<<contentStartIdx+contentLength<<
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

    kDebug()<<"attempting to parse "<<out;

    QChar c;

    int numResults=0;

    QList<Cantor::Result*> results;
    while(idx<out.size())
    {
        skipWhitespaces(&idx, out);

        //first read the part not enclosed in tags. it most likely belongs to an error message
        int idx1=out.indexOf("<cantor-prompt>", idx);
        int idx2=out.indexOf("<cantor-result>", idx);

        idx1=(idx1==-1) ? out.size():idx1;
        idx2=(idx2==-1) ? out.size():idx2;
        int newIdx=qMin(idx1, idx2);

        if(newIdx>idx)
        {
            const QString& err=out.mid(idx, newIdx-idx);
            if(!err.isEmpty())
                m_gotErrorContent=true;
            m_errorBuffer+=err;
            kDebug()<<"the unmatched part of the output is: "<<err;
            idx=newIdx;
        }

        const QStringRef& tag=readXmlOpeningTag(&idx, out);

        if(tag=="cantor-result")
        {
            kDebug()<<"got a result";

            Cantor::Result* result=parseResult(&idx, out);
            results<<result;
            numResults++;

            kDebug()<<"got "<<numResults<<"th result.";

        }else if (tag=="cantor-prompt")
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
                    if(type=="/cantor-result")
                        break;
                    const QStringRef& content=readXmlTagContent(&idx, out, type);

                    if(type=="cantor-text")
                        text=content;
                    else if(type=="cantor-latex")
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

                if(!m_errorBuffer.trimmed().isEmpty())
                {
                    //Replace < and > with their html code, so they won't be confused as html tags
                    m_errorBuffer.replace( '<' , "&lt;");
                    m_errorBuffer.replace( '>' , "&gt;");

                    if(command().startsWith(":lisp")||command().startsWith(":lisp-quiet"))
                    {
                        foreach(Cantor::Result* result, results)
                        {
                            if(result->type()==Cantor::TextResult::Type)
                                m_errorBuffer.prepend(dynamic_cast<Cantor::TextResult*>(result)->plain()+"\n");
                            else if(result->type()==Cantor::LatexResult::Type)
                                m_errorBuffer.prepend(dynamic_cast<Cantor::LatexResult*>(result)->plain()+"\n");
                        }

                        Cantor::TextResult* result=new Cantor::TextResult(m_errorBuffer);
                        setResult(result);
                        setStatus(Cantor::Expression::Done);
                    }else
                    if(m_isHelpRequest) //Help Messages are also provided in the errorBuffer.
                    {
                        Cantor::HelpResult* result=new Cantor::HelpResult(m_errorBuffer);
                        setResult(result);

                        setStatus(Cantor::Expression::Done);
                    }else
                    {
                        foreach(Cantor::Result* result, results)
                        {
                            kDebug()<<"result: "<<result->toHtml();
                            if(result->type()==Cantor::TextResult::Type)
                                m_errorBuffer.prepend(dynamic_cast<Cantor::TextResult*>(result)->plain()+"\n");
                            else if(result->type()==Cantor::LatexResult::Type)
                                m_errorBuffer.prepend(dynamic_cast<Cantor::LatexResult*>(result)->plain()+"\n");
                        }

                        kDebug()<<"errorBuffer: "<<m_errorBuffer;


                        setErrorMessage(m_errorBuffer.trimmed());
                        if(m_gotErrorContent)
                            setStatus(Cantor::Expression::Error);
                        else
                            setStatus(Cantor::Expression::Done);
                    }
                }
                else
                {
                    //if we got an error message, but also a result, lets just
                    //assume that it was just a warning, as obviously something worked
                    if(errorMessage().isEmpty()||results.size()>0)
                    {
                        setResults(results);
                        setStatus(Cantor::Expression::Done);
                    }
                    else
                    {
                        if(results.size()==0)
                            setStatus(Cantor::Expression::Error);

                    }
                }

                out=out.mid(idx);
                idx=0;

                return true;
            }
        }else
        {
            kDebug()<<"unknown tag"<<tag;
        }
    }

    //show partial result
    setResults(results);
    return false;
}

Cantor::Result* MaximaExpression::parseResult(int* idx, QString& out)
{
    bool isLatexComplete=false;
    QString latex;
    QString text;

    while(*idx<out.size())
    {
        bool isComplete;
        const QStringRef& type=readXmlOpeningTag(idx, out);

        if(type=="/cantor-result")
            break;

        const QStringRef& content=readXmlTagContent(idx, out, type, &isComplete);

        if(type=="cantor-text")
            text=content.toString().trimmed();
        else if(type=="cantor-latex")
        {
            isLatexComplete=isComplete;
            latex=content.toString().trimmed();
        }
    }

    //Replace < and > with their html code, so they won't be confused as html tags
    text.replace( '<' , "&lt;");
    text.replace( '>' , "&gt;");

    QRegExp outputPromptRegexp=QRegExp('^'+MaximaSession::MaximaOutputPrompt.pattern());
    int idxOfPrompt=outputPromptRegexp.indexIn(text);
    text.remove(idxOfPrompt, outputPromptRegexp.matchedLength());

    //find the number if this output in the MaximaOutputPrompt
    QString prompt=outputPromptRegexp.cap(0).trimmed();
    bool ok;
    QString idString=prompt.mid(3, prompt.length()-4);
    int id=idString.toInt(&ok);
    if(ok)
        setId(id);
    else
        setId(-1);
    kDebug()<<"prompt: "<<prompt<<" id: "<<id;

    if(m_tempFile)
    {
        QTimer::singleShot(500, this, SLOT(imageChanged()));
    }

    Cantor::TextResult* result=0;

    if(m_isPlot&&text.trimmed()=="\"\"")
    {
        kDebug()<<"result is a plot!";
        result=new Cantor::TextResult(i18n("Waiting for Image..."), "cantor-internal-plot");
        return result;
    }

    //if the <latex> element wasn't read completely, there
    //is no point in trying to render it. Use text for
    //incomplete results.
    if(!isLatexComplete
       ||latex.trimmed().isEmpty()
       ||m_isHelpRequest||isInternal())
    {
        kDebug()<<"using text";
        result=new Cantor::TextResult(text);
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

        latex=latex.mid(i+1);

        //no need to render empty latex.
        if(latex.trimmed().isEmpty())
        {
            if(m_isPlot)
            {
                kDebug()<<"result is a plot!";
                result=new Cantor::TextResult(i18n("Waiting for Image..."), "cantor-internal-plot");
                return result;
            }
            else
                result=0;
        }else
        {
            latex.prepend("\\begin{eqnarray*}\n");
            latex.append("\n\\end{eqnarray*}");
            result=new Cantor::TextResult(latex, text);
            result->setFormat(Cantor::TextResult::LatexFormat);
        }
    }


    return result;
}

void MaximaExpression::parseError(const QString& out)
{
    m_errorBuffer.append(out);
}

void MaximaExpression::imageChanged()
{
    kDebug()<<"the temp image has changed";
    if(m_tempFile->size()>0)
    {
        for(int i=0;i<results().size();i++)
        {
            Cantor::Result* result=results().at(i);
            if(result->type()==Cantor::TextResult::Type &&static_cast<Cantor::TextResult*>(result)->plain()=="cantor-internal-plot")
            {
#ifdef WITH_EPS
                setResult( new Cantor::EpsResult( KUrl(m_tempFile->fileName()) ) , i);
#else
                setResult( new Cantor::ImageResult( KUrl(m_tempFile->fileName()) ) , i);
#endif
                setStatus(Cantor::Expression::Done);
            }
        }
    }
}


QString MaximaExpression::additionalLatexHeaders()
{
    return QString();
}

#include "maximaexpression.moc"
