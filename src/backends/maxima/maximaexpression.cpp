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

#define ASK_TIME 100

MaximaExpression::MaximaExpression( Cantor::Session* session, MaximaExpression::Type type ) : Cantor::Expression(session)
{
    kDebug();
    m_type=type;
    m_tempFile=0;
    m_latexFailed=false;
    //this is a timer that is triggered if we got output without an output prompt
    //and no new input prompt for some time, so we assume that it's because maxima
    //is asking for information
    m_askTimer=new QTimer(this);
    m_askTimer->setSingleShot(true);
    connect(m_askTimer, SIGNAL(timeout()), this, SLOT(askForInformation()));
}

MaximaExpression::~MaximaExpression()
{

}

void MaximaExpression::setType(Type type)
{
    m_type=type;
}

MaximaExpression::Type MaximaExpression::type()
{
    return m_type;
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

    m_onStdoutStroke=false;
    m_outputPrefix.clear();
    m_output.clear();
    m_errCache.clear();

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

    if(type()==MaximaExpression::NormalExpression)
        dynamic_cast<MaximaSession*>(session())->appendExpressionToQueue(this);
    else
        dynamic_cast<MaximaSession*>(session())->appendExpressionToHelperQueue(this);
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

void MaximaExpression::parseOutput(const QString& text)
{
    if(type()==MaximaExpression::TexExpression)
    {
        parseTexResult(text);
    }else
    {
        parseNormalOutput(text);
    }
}

void MaximaExpression::parseNormalOutput(const QString& text)
{
    //new information arrived, stop the question timeout.
    //restart it later after the new information was parsed
    m_askTimer->stop();

    QString output=text;

    kDebug()<<"parsing: "<<output;

    //Remove all whitespaces and newlines in the occurring prompts, to simplify parsing later
    int index=-1;
    while( (index=MaximaSession::MaximaOutputPrompt.indexIn(output, index+1) )>=0 )
    {
        QString newPrompt=MaximaSession::MaximaOutputPrompt.cap(0);
        newPrompt.remove(QRegExp("\\s"));
        output.replace(index, MaximaSession::MaximaOutputPrompt.matchedLength(), newPrompt);
    }

    bool couldBeQuestion=false;
    const QStringList lines=output.split('\n');
    //A regexp matching for OutputPrompt, but allowing an arbitrary number of spaces at the beginning
    const QRegExp outputPromptRegexp(QString("\\s*%1").arg(MaximaSession::MaximaOutputPrompt.pattern()));

    foreach(QString line, lines) // krazy:exclude=foreach
    {
        if(line.endsWith('\r'))
            line.chop(1);
        if(line.isEmpty())
            continue;

        if (MaximaSession::MaximaPrompt.exactMatch(line.trimmed()))
        {
            evalFinished();
            m_onStdoutStroke=false;
            couldBeQuestion=false;
        }else if(outputPromptRegexp.indexIn(line)==0)
        {
            //find the number if this output in the MaximaOutputPrompt
            QString prompt=outputPromptRegexp.cap(0).trimmed();
            QString id=prompt.mid(3, prompt.length()-4);
            setId(id.toInt());

            line.remove(MaximaSession::MaximaOutputPrompt);
            //we got regular output. this means no error occurred,
            //prepend the error Buffer to the output Buffer, as
            //for example when display2d:true, the ouputPrompt doesn't
            //need to be in the first line
            if(m_output.isEmpty())
            {
                m_outputPrefix=m_errCache;
                m_errCache.clear();
            }

            //append the line to the output cache, but
            //only it isn't false, as a line only
            //containing "%O1 false" means that this
            //output can be ignored
            if(line.trimmed() != "false" )
                m_output.append(line+'\n');

            m_onStdoutStroke=true;
            couldBeQuestion=false;
        }else if (m_onStdoutStroke)
        {
            m_output.last()+=line+'\n';
        }
        else
        {
            kDebug()<<"got something";
            m_errCache+=line+'\n';
            m_onStdoutStroke=false;
            couldBeQuestion=true;
        }
    }

    //wait a bit if another chunk of data comes in, containing the needed PROMPT
    if(couldBeQuestion)
    {
            m_askTimer->stop();
            m_askTimer->start(ASK_TIME);
    }

}

void MaximaExpression::askForInformation()
{
    emit needsAdditionalInformation(m_errCache.trimmed());
    m_outputPrefix.clear();
    m_output.clear();
    m_onStdoutStroke=false;
    m_errCache.clear();
}

void MaximaExpression::parseTexResult(const QString& text)
{
    const QString& output=text.trimmed();

    m_errCache+=output;

    kDebug()<<"parsing "<<text;
    kDebug()<<"prefix: "<<m_outputPrefix;

    //If we haven't got the Input prompt yet, postpone the parsing
    if(!m_errCache.contains(MaximaSession::MaximaPrompt))
        return;

    QString completeLatex;
    if(!m_outputPrefix.isEmpty())
        completeLatex="{ \\small \\noindent "+m_outputPrefix.trimmed().replace("\n", "\\newline ")+"} \n";
    while(m_errCache.contains(MaximaSession::MaximaOutputPrompt))
    {
        kDebug()<<"got prompt"<<m_errCache;
        int pos=m_errCache.indexOf(MaximaSession::MaximaOutputPrompt);
        QString latex=m_errCache.mid(0, pos).trimmed();
        if(latex.startsWith(QLatin1String("$$")))
        {
            latex=latex.mid(2);
            latex.prepend("\\begin{eqnarray*}");
        }
        if(latex.endsWith(QLatin1String("$$")))
        {
            latex.chop(2);
            latex.append("\\end{eqnarray*}");
        }
        completeLatex+=latex+'\n';

        m_errCache.remove(0, (m_errCache.indexOf("false", pos)+5));
    }

    if(!completeLatex.isEmpty())
    {
        kDebug()<<"latex: "<<completeLatex;
        Cantor::TextResult* result=new Cantor::TextResult(completeLatex);
        result->setFormat(Cantor::TextResult::LatexFormat);

        m_outputPrefix.clear();
        setResult(result);
    }else
    {
        //Running TeX seems to have failed...
        m_latexFailed=true;
    }

    setStatus(Cantor::Expression::Done);

}

void MaximaExpression::evalFinished()
{
    kDebug()<<"evaluation finished";
    kDebug()<<"out: "<<m_outputPrefix;
    kDebug()<<"out: "<<m_output;
    kDebug()<<"err: "<<m_errCache;

    QString text=m_outputPrefix;
    text+=m_output.join(QLatin1String("\n"));
    if(!m_isHelpRequest&&!session()->isTypesettingEnabled()) //don't screw up Maximas Ascii-Art
        text.replace(' ', "&nbsp;");

    //Replace < and > with their html code, so they won't be confused as html tags
    text.replace( '<' , "&lt;");
    text.replace( '>' , "&gt;");

    Cantor::TextResult* result;

    if(m_tempFile)
    {
        QTimer::singleShot(500, this, SLOT(imageChanged()));
        if(m_errCache.trimmed().isEmpty()&&m_output.join(" ").trimmed().isEmpty())
        {
            return;
        }
    }

    if(m_isHelpRequest)
    {
        result=new Cantor::HelpResult(text);
        setResult(result);
        setStatus(Cantor::Expression::Done);
    }
    else
    {
        result=new Cantor::TextResult(text);

        setResult(result);

        if(!m_errCache.isEmpty())
        {
            setErrorMessage(m_errCache);
            setStatus(Cantor::Expression::Error);
        }
        else
        {
            setStatus(Cantor::Expression::Done);
        }
    }
}

bool MaximaExpression::needsLatexResult()
{
    bool needsLatex=!m_latexFailed&&
        session()->isTypesettingEnabled() &&
        status()!=Cantor::Expression::Error &&
        finishingBehavior()==Cantor::Expression::DoNotDelete;

    if (result()&&result()->type()==Cantor::TextResult::Type&&result()->data().toString()!="false" )
       return needsLatex && dynamic_cast<Cantor::TextResult*>(result())->format()!=Cantor::TextResult::LatexFormat;
    else
        return false;
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

QStringList MaximaExpression::output()
{
    return m_output;
}

#include "maximaexpression.moc"
