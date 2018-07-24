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

#include <QDir>
#include <QTemporaryFile>

#include <KLocalizedString>
#include <QDebug>
#include <QTimer>
#include <QRegExp>
#include <QChar>
#include <QUrl>

MaximaExpression::MaximaExpression( Cantor::Session* session ) : Cantor::Expression(session),
    m_tempFile(nullptr),
    m_isHelpRequest(false),
    m_isPlot(false),
    m_gotErrorContent(false)
{
}

void MaximaExpression::evaluate()
{
    setStatus(Cantor::Expression::Computing);

    //until we get the real output Id from maxima, set it to invalid
    setId(-1);

    m_isHelpRequest=false;
    m_isPlot=false;
    m_gotErrorContent=false;
    if(m_tempFile)
        m_tempFile->deleteLater();
    m_tempFile=nullptr;
    //check if this is a ?command
    if(command().startsWith(QLatin1Char('?'))||command().startsWith(QLatin1String("describe("))||command().startsWith(QLatin1String("example(")))
        m_isHelpRequest=true;

    if(command().contains(QRegExp(QLatin1String("(?:plot2d|plot3d|contour_plot)\\s*\\([^\\)]"))) && MaximaSettings::self()->integratePlots() && !command().contains(QLatin1String("ps_file")))
    {
        m_isPlot=true;
#ifdef WITH_EPS
        m_tempFile=new QTemporaryFile(QDir::tempPath() + QLatin1String("/cantor_maxima-XXXXXX.eps" ));
#else
        m_tempFile=new QTemporaryFile(QDir::tempPath() + QLatin1String("/cantor_maxima-XXXXXX.png"));
#endif
        m_tempFile->open();

        disconnect(&m_fileWatch, &KDirWatch::dirty, this, &MaximaExpression::imageChanged);
        m_fileWatch.addFile(m_tempFile->fileName());
        connect(&m_fileWatch, &KDirWatch::dirty, this, &MaximaExpression::imageChanged);
    }

    const QString& cmd=command();

    bool isComment = true;
    int commentLevel = 0;
    bool inString = false;
    for (int i = 0; i < cmd.size(); ++i) {
        if (cmd[i] == QLatin1Char('\\')) {
            ++i; // skip the next character
            if (commentLevel == 0 && !inString) {
                isComment = false;
            }
        } else if (cmd[i] == QLatin1Char('"') && commentLevel == 0) {
            inString = !inString;
            isComment = false;
        } else if (cmd.mid(i,2) == QLatin1String("/*") && !inString) {
            ++commentLevel;
            ++i;
        } else if (cmd.mid(i,2) == QLatin1String("*/") && !inString) {
            if (commentLevel == 0) {
                qDebug() << "Comments mismatched!";
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
        qDebug() << "Comments mismatched!";
        setErrorMessage(i18n("Error: Too many /*"));
        setStatus(Cantor::Expression::Error);
        return;
    }
    if (inString) {
        qDebug() << "String not closed";
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
            qDebug()<<"plotting without tempFile";
            return QString();
        }
        QString fileName = m_tempFile->fileName();

#ifdef WITH_EPS
        const QString psParam=QLatin1String("[gnuplot_ps_term_command, \"set size 1.0,  1.0; set term postscript eps color solid \"]");
        const QString plotParameters = QLatin1String("[ps_file, \"")+ fileName+QLatin1String("\"],")+psParam;
#else
        const QString plotParameters = QLatin1String("[gnuplot_term, \"png size 500,340\"], [gnuplot_out_file, \"")+fileName+QLatin1String("\"]");

#endif
        cmd.replace(QRegExp(QLatin1String("((plot2d|plot3d|contour_plot)\\s*\\(.*)\\)([;\n]|$)")), QLatin1String("\\1, ")+plotParameters+QLatin1String(");"));

    }

    if (!cmd.endsWith(QLatin1Char('$')))
    {
        if (!cmd.endsWith(QLatin1String(";")))
            cmd+=QLatin1Char(';');
    }

    //replace all newlines with spaces, as maxima isn't sensitive about
    //whitespaces, and without newlines the whole command
    //is executed at once, without outputting an input
    //prompt after each line
    cmd.replace(QLatin1Char('\n'), QLatin1Char(' '));

    //lisp-quiet doesn't print a prompt after the command
    //is completed, which causes the parsing to hang.
    //replace the command with the non-quiet version
    cmd.replace(QRegExp(QLatin1String("^:lisp-quiet")), QLatin1String(":lisp"));

    return cmd;
}

void MaximaExpression::forceDone()
{
    qDebug()<<"forcing Expression state to DONE";
    setResult(nullptr);
    setStatus(Cantor::Expression::Done);
}

void MaximaExpression::addInformation(const QString& information)
{
    qDebug()<<"adding information";
    QString inf=information;
    if(!inf.endsWith(QLatin1Char(';')))
        inf+=QLatin1Char(';');
    Cantor::Expression::addInformation(inf);

    dynamic_cast<MaximaSession*>(session())->sendInputToProcess(inf+QLatin1Char('\n'));
}


//The maxima backend is modified, so that it outputs
//xml-style tags around outputs, input prompts etc.
//the following are some simple helper functions to faciliate parsing
inline void skipWhitespaces(int* idx, const QString& txt)
{
    for(;*idx < txt.size() && (txt[*idx]).isSpace();++(*idx));
}

QStringRef readXmlOpeningTag(int* idx, const QString& txt, bool* isComplete=nullptr)
{
    qDebug()<<"trying to read an opening tag";

    if (*idx >= txt.size())
        return QStringRef();

    skipWhitespaces(idx, txt);

    if(isComplete)
        *isComplete=false;

    if(txt[*idx]!=QLatin1Char('<'))
    {
        qDebug()<<"This is NOT AN OPENING TAG."<<endl
                <<"Dropping everything until next opening; This starts with a " <<txt[*idx];
        int newIdx=txt.indexOf(QLatin1Char('<'), *idx);
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
    while(*idx<txt.size())
    {
        const QChar c=txt[*idx];
        ++(*idx);

        if(c==QLatin1Char('>'))
        {
            if(isComplete)
                *isComplete=true;
            break;
        }else
            length++;
    }

    return QStringRef(&txt, startIndex, length);
}

QStringRef readXmlTagContent(int* idx, const QString& txt, const QStringRef& name, bool* isComplete=nullptr)
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

        if(c==QLatin1Char('/')&&(*idx)>0&&txt[(*idx)-1]==QLatin1Char('<'))
        {
            //remove the opening <
            contentLength--;
            currentTagStartIdx=*idx+1;
            currentTagLength=0;
            readingClosingTag=true;
        }
        else if(readingClosingTag)
        {
            if(c==QLatin1Char('>'))
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
        qDebug()<<"something is wrong with the content-length "<<contentStartIdx+contentLength<<
            " vs: "<<txt.size();
    }
    return QStringRef(&txt,contentStartIdx, contentLength);
}

bool MaximaExpression::parseOutput(QString& out)
{
    enum ParserStatus{ReadingOpeningTag, ReadingClosingTag, ReadingText};
    int idx=0;
    int numResults=0;
    QString textBuffer;
    QString latexBuffer;
    QString errorBuffer;

    Cantor::Result* result=nullptr;
    while(idx<out.size())
    {
        skipWhitespaces(&idx, out);

        //first read the part not enclosed in tags. it most likely belongs to an error message
        int idx1=out.indexOf(QLatin1String("<cantor-prompt>"), idx);
        int idx2=out.indexOf(QLatin1String("<cantor-result>"), idx);

        idx1=(idx1==-1) ? out.size():idx1;
        idx2=(idx2==-1) ? out.size():idx2;
        int newIdx=qMin(idx1, idx2);

        if(newIdx>idx)
        {
            const QString& err=out.mid(idx, newIdx-idx);
            if(!err.isEmpty())
                m_gotErrorContent=true;
            errorBuffer+=err;
            qDebug()<<"the unmatched part of the output is: "<<err;
            idx=newIdx;
        }

        const QStringRef& tag=readXmlOpeningTag(&idx, out);

        if(tag==QLatin1String("cantor-result"))
        {
            qDebug()<<"got a result";

            if(numResults>0)
            {
                textBuffer.append(QLatin1String("\n"));
                latexBuffer.append(QLatin1String("\n"));
            }

            result=parseResult(&idx, out, textBuffer, latexBuffer);
            numResults++;

            qDebug()<<"got "<<numResults<<"th result.";

        }else if (tag==QLatin1String("cantor-prompt"))
        {
            qDebug()<<"i got a prompt: "<<idx;

            skipWhitespaces(&idx, out);

            //We got a child tag
            if(out[idx]==QLatin1Char('<'))
            {
                const QStringRef& childTag=readXmlOpeningTag(&idx, out);
                qDebug()<<"got an information request!"<<childTag;

                QStringRef text;
                QStringRef latex;
                while(idx<out.size())
                {
                    const QStringRef& type=readXmlOpeningTag(&idx, out);
                    qDebug()<<"its a "<<type;
                    if(type==QLatin1String("/cantor-result"))
                        break;
                    const QStringRef& content=readXmlTagContent(&idx, out, type);

                    if(type==QLatin1String("cantor-text"))
                        text=content;
                    else if(type==QLatin1String("cantor-latex"))
                        latex=content;

                    qDebug()<<"content: "<<content;
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
                readXmlTagContent(&idx, out, tag, &isComplete);

                if(!isComplete)
                    return false;

                m_errorBuffer+=errorBuffer;
                if(!m_errorBuffer.trimmed().isEmpty())
                {
                    //Replace < and > with their html code, so they won't be confused as html tags
                    m_errorBuffer.replace( QLatin1Char('<') , QLatin1String("&lt;"));
                    m_errorBuffer.replace( QLatin1Char('>') , QLatin1String("&gt;"));

                    if(command().startsWith(QLatin1String(":lisp"))||command().startsWith(QLatin1String(":lisp-quiet")))
                    {
                        if(result)
                        {
                            if(result->type()==Cantor::TextResult::Type)
                                m_errorBuffer.prepend(dynamic_cast<Cantor::TextResult*>(result)->plain()+QLatin1String("\n"));
                            else if(result->type()==Cantor::LatexResult::Type)
                                m_errorBuffer.prepend(dynamic_cast<Cantor::LatexResult*>(result)->plain()+QLatin1String("\n"));
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
                        if(result)
                        {
                            qDebug()<<"result: "<<result->toHtml();
                            if(result->type()==Cantor::TextResult::Type)
                                m_errorBuffer.prepend(dynamic_cast<Cantor::TextResult*>(result)->plain()+QLatin1String("\n"));
                            else if(result->type()==Cantor::LatexResult::Type)
                                m_errorBuffer.prepend(dynamic_cast<Cantor::LatexResult*>(result)->plain()+QLatin1String("\n"));
                        }

                        qDebug()<<"errorBuffer: "<<m_errorBuffer;


                        setErrorMessage(m_errorBuffer.trimmed());
                        if(m_gotErrorContent)
                            setStatus(Cantor::Expression::Error);
                        else
                            setStatus(Cantor::Expression::Done);
                    }
                }
                else
                {
                    //if we got an error message, but also a result, lets just+
                    //assume that it was just a warning, as obviously something worked
                    if(errorMessage().isEmpty()||result!=nullptr)
                    {
                        setResult(result);
                        setStatus(Cantor::Expression::Done);
                    }
                    else
                    {
                        if(!result)
                            setStatus(Cantor::Expression::Error);

                    }
                }

                out=out.mid(idx);
                idx=0;

                return true;
            }
        }else
        {
            qDebug()<<"unknown tag"<<tag;
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

        if(type==QLatin1String("/cantor-result"))
            break;

        const QStringRef& content=readXmlTagContent(idx, out, type, &isComplete);

        if(type==QLatin1String("cantor-text"))
            text=content.toString().trimmed();
        else if(type==QLatin1String("cantor-latex"))
        {
            isLatexComplete=isComplete;
            latex=content.toString().trimmed();
        }
    }

    //Replace < and > with their html code, so they won't be confused as html tags
    text.replace( QLatin1Char('<') , QLatin1String("&lt;"));
    text.replace( QLatin1Char('>') , QLatin1String("&gt;"));

    QRegExp outputPromptRegexp=QRegExp(QLatin1Char('^')+MaximaSession::MaximaOutputPrompt.pattern());
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
    qDebug()<<"prompt: "<<prompt<<" id: "<<id;

    if(m_tempFile)
    {
        QTimer::singleShot(500, this, SLOT(imageChanged()));
    }

    Cantor::TextResult* result=nullptr;

    //if this is not the first result, prepend the results
    //found in the earlier tags.
    textBuffer.append(text);

    //if the <latex> element wasn't read completely, there
    //is no point in trying to render it. Use text for
    //incomplete results.
    if(!isLatexComplete
       ||(latexBuffer.trimmed().isEmpty()&&latex.isEmpty())
       ||m_isHelpRequest||isInternal())
    {
        qDebug()<<"using text";
        result=new Cantor::TextResult(textBuffer);
    }else
    {
        qDebug()<<"using latex";
        //strip away the latex code for the label.
        //it is contained in an \mbox{} call
        int i;
        int pcount=0;
        for(i=latex.indexOf(QLatin1String("\\mbox{"))+5;i<latex.size();i++)
        {
            if(latex[i]==QLatin1Char('{'))
                pcount++;
            else if(latex[i]==QLatin1Char('}'))
                pcount--;

            if(pcount==0)
                break;
        }

        latex=latex.mid(i+1);

        //no need to render empty latex.
        if(latexBuffer.trimmed().isEmpty()&&latex.trimmed().isEmpty())
        {
            if(m_isPlot)
                result=new Cantor::TextResult(i18n("Waiting for Image..."));
            else
                result=nullptr;
        }else
        {
            latex.prepend(QLatin1String("\\begin{eqnarray*}\n"));
            latex.append(QLatin1String("\n\\end{eqnarray*}"));
            latexBuffer.append(latex);
            result=new Cantor::TextResult(latexBuffer, textBuffer);
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
    qDebug()<<"the temp image has changed";
    if(m_tempFile->size()>0)
    {
#ifdef WITH_EPS
        setResult( new Cantor::EpsResult( QUrl::fromLocalFile(m_tempFile->fileName()) ) );
#else
        setResult( new Cantor::ImageResult( QUrl::fromLocalFile(m_tempFile->fileName()) ) );
#endif
        setStatus(Cantor::Expression::Done);
    }
}


QString MaximaExpression::additionalLatexHeaders()
{
    return QString();
}

