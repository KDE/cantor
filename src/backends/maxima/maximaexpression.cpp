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

QString readXmlOpeningTag(int* idx, const QString& txt, bool* isComplete=0)
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
            return QString::null;
        }else
        {
            (*idx)=newIdx+1;
        }

    }else
    {
        ++ (*idx);
    }

    QString name;
    while(*idx<txt.size())
    {
        const QChar c=txt[*idx];
        ++(*idx);

        if(c=='>')
        {
            if(isComplete)
                *isComplete=true;
            return name;
        }else
            name+=c;
    }

    return name;
}

QString readXmlTagContent(int* idx, const QString& txt, const QString& name, bool* isComplete=0)
{
    QString content;
    QString currentTagName;
    bool readingClosingTag=false;

    if(isComplete)
        *isComplete=false;

    while(*idx<txt.size())
    {
        const QChar c=txt[*idx];

        if(c=='/'&&(*idx)>0&&txt[(*idx)-1]=='<')
        {
            //remove the opening <
            content.chop(1);
            readingClosingTag=true;
        }
        else if(readingClosingTag)
        {
            if(c=='>')
            {
                kDebug()<<"a tag just closed: "<<currentTagName;
                if(currentTagName==name)
                {
                    //eat up the closing >
                    ++(*idx);
                    if(isComplete)
                        (*isComplete)=true;
                    break;
                }
                else
                {
                    content+="</"+currentTagName;
                    currentTagName.clear();
                }

                readingClosingTag=false;
            }else
                currentTagName+=c;
        }
        else
            content+=c;


        ++(*idx);

    }

    return content;
}

bool MaximaExpression::parseOutput(QString& out)
{
    enum ParserStatus{ReadingOpeningTag, ReadingClosingTag, ReadingText};
    int idx=0;
    ParserStatus status;
    QString tagName;
    QString textBuffer;
    QString errorBuffer;
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


    errorBuffer=out.left(idx);
    kDebug()<<"the unmatched part of the output is: "<<errorBuffer;

    while(idx<out.size())
    {
        kDebug()<<"1)idx: "<<idx;
        skipWhitespaces(&idx, out);
        kDebug()<<"2)idx: "<<idx;

        QString tag=readXmlOpeningTag(&idx, out);

        kDebug()<<"big loop read " <<tag;
        if(tag=="result")
        {
            kDebug()<<"hey, I got a result";
            parseResult(&idx, out);

        }else if (tag=="prompt")
        {
            kDebug()<<"i got a prompt: "<<idx;

            skipWhitespaces(&idx, out);

            //We got a child tag
            if(out[idx]=='<')
            {
                const QString& tag=readXmlOpeningTag(&idx, out);
                kDebug()<<"got an information request!"<<tag;

                QString text;
                QString latex;
                while(idx<out.size())
                {
                    const QString type=readXmlOpeningTag(&idx, out);
                    kDebug()<<"its a "<<type;
                    if(type=="/result")
                        break;
                    const QString& content=readXmlTagContent(&idx, out, type);

                    if(type=="text")
                        text=content;
                    else if(type=="latex")
                        latex=content;

                    kDebug()<<"content: "<<content;
                }

                bool isComplete;
                //readup the rest of the element and discard it
                readXmlTagContent(&idx, out, "prompt", &isComplete);

                if(!isComplete)
                    return false;

                out=out.mid(idx);
                idx=0;

                //send out the information request
                emit needsAdditionalInformation(text);

                return true;

            }else //got a regular prompt. Just read it all
            {
                bool isComplete;
                const QString& content=readXmlTagContent(&idx, out, "prompt", &isComplete);

                if(!isComplete)
                    return false;

                out=out.mid(idx);
                idx=0;

                if(!errorBuffer.trimmed().isEmpty())
                {
                    //Replace < and > with their html code, so they won't be confused as html tags
                    errorBuffer.replace( '<' , "&lt;");
                    errorBuffer.replace( '>' , "&gt;");

                    setErrorMessage(errorBuffer.trimmed());
                    setStatus(Cantor::Expression::Error);
                }
                else
                {
                    setStatus(Cantor::Expression::Done);
                }

                return true;
            }
        }else
        {
            kDebug()<<"tag: "<<tag;
            kDebug()<<"WTF are you doing?";
        }
    }

    return false;
}

void MaximaExpression::parseResult(int* idx, QString& out)
{
    QString text;
    QString latex;

    bool isLatexComplete;

    while(*idx<out.size())
    {
        bool isComplete;
        const QString type=readXmlOpeningTag(idx, out);
        kDebug()<<"its a "<<type;
        if(type=="/result")
            break;

        const QString& content=readXmlTagContent(idx, out, type, &isComplete);

        if(type=="text")
            text=content.trimmed();
        else if(type=="latex")
        {
            isLatexComplete=isComplete;
            latex=content.trimmed();
        }

        kDebug()<<"content: "<<content;
    }

    //Replace < and > with their html code, so they won't be confused as html tags
    text.replace( '<' , "&lt;");
    text.replace( '>' , "&gt;");

    if(m_tempFile)
    {
        QTimer::singleShot(500, this, SLOT(imageChanged()));
    }

    Cantor::TextResult* result=0;
    if(m_isHelpRequest)
    {
        kDebug()<<"its a help thing!";
        result=new Cantor::HelpResult(text);
        setResult(result);
    }
    else
    {
        //if the <latex> element wasn't read completely, there
        //is no point in trying to render it. Use text for
        //incomplete results.
        if(!isLatexComplete||latex.trimmed().isEmpty())
        {
            result=new Cantor::TextResult(text);
        }else
        {
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
            if(latex.trimmed().isEmpty())
            {
                if(m_isPlot)
                    result=new Cantor::TextResult(i18n("Loading Image..."));
                else
                    result=0;
            }else
            {
                latex.prepend("\\begin{eqnarray*}\n");
                latex.append("\n\\end{eqnarray*}\n");
                result=new Cantor::TextResult(latex);
                result->setFormat(Cantor::TextResult::LatexFormat);
            }
        }

        if(result)
            setResult(result);
    }
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
