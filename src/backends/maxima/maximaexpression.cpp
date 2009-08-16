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

MaximaExpression::MaximaExpression( MathematiK::Session* session ) : MathematiK::Expression(session)
{
    kDebug();
    m_tempFile=0;
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

void MaximaExpression::evaluate()
{
    kDebug()<<"evaluating "<<command();
    setStatus(MathematiK::Expression::Computing);

    m_isHelpRequest=false;
    if(m_tempFile)
        m_tempFile->deleteLater();
    m_tempFile=0;
    //check if this is a ?command
    if(command().startsWith('?')||command().startsWith(QLatin1String("describe("))||command().startsWith(QLatin1String("example(")))
        m_isHelpRequest=true;

    m_onStdoutStroke=false;
    m_outputCache.clear();
    m_errCache.clear();

    if(command().contains(QRegExp("^plot2d|plot3d")) && MaximaSettings::self()->integratePlots() && !command().contains("psfile"))
    {
        m_isPlot=true;
        m_tempFile=new KTemporaryFile();
        m_tempFile->setPrefix( "mathematik_maxima-" );
        m_tempFile->setSuffix( ".eps" );
        m_tempFile->open();

        m_fileWatch.addFile(m_tempFile->fileName());
        connect(&m_fileWatch, SIGNAL(dirty(const QString&)), this, SLOT(imageChanged()));
    }

    //if the whole command consists of a command, drop it
    static const QRegExp commentRegExp("^/\\*.*\\*/$");
    if(commentRegExp.exactMatch(command()))
       return;

    dynamic_cast<MaximaSession*>(session())->appendExpressionToQueue(this);
}

void MaximaExpression::interrupt()
{
    kDebug()<<"interrupting";
    dynamic_cast<MaximaSession*>(session())->sendSignalToProcess(2);
}

QString MaximaExpression::internalCommand()
{
    QString cmd=command();

    if(m_isPlot)
    {
        QString fileName = m_tempFile->fileName();

        QString psParam="[gnuplot_ps_term_command, \"set size 1.0,  1.0; set term postscript eps color solid \"]";
        QString plotParameters = "[psfile, \""+ fileName+"\"],"+psParam;
        cmd.insert(cmd.lastIndexOf(')'), ','+plotParameters);
    }

    if (!cmd.endsWith('$'))
    {
        if (!cmd.endsWith(QLatin1String(";")))
            cmd+=';';
    }

    return cmd;
}

void MaximaExpression::addInformation(const QString& information)
{
    kDebug()<<"adding information";
    QString inf=information;
    if(!inf.endsWith(';'))
        inf+=';';
    MathematiK::Expression::addInformation(inf);

    dynamic_cast<MaximaSession*>(session())->sendInputToProcess(inf+'\n');
}

void MaximaExpression::parseOutput(const QString& text)
{
    //new information arrived, stop the question timeout.
    //restart it later after the new information was parsed
    m_askTimer->stop();

    QString output=text;

    kDebug()<<"parsing: "<<output;

    ///if(output.contains("maxima: "));
    //output=output.mid(6).trimmed();

    if(m_tempFile)
    {
        QTimer::singleShot(500, this, SLOT(imageChanged()));
        return;
    }

    bool couldBeQuestion=false;
    const QStringList lines=output.split('\n');
    foreach(QString line, lines)
    {
        if(line.endsWith('\r'))
            line.chop(1);
        if(line.isEmpty())
            continue;

        if (line.indexOf(MaximaSession::MaximaPrompt)!=-1)
        {
            evalFinished();
            m_onStdoutStroke=false;
            couldBeQuestion=false;
        }else if(line.indexOf(MaximaSession::MaximaOutputPrompt)==0||m_onStdoutStroke)
        {
            //find the number if this output in the MaximaOutputPrompt
            QString prompt=line.mid(MaximaSession::MaximaOutputPrompt.indexIn(line), MaximaSession::MaximaOutputPrompt.matchedLength()).trimmed();
            QString id=prompt.mid(3, prompt.length()-4);
            setId(id.toInt());

            line.remove(MaximaSession::MaximaOutputPrompt);
            //we got regular output. this means no error occurred,
            //prepend the error Buffer to the output Buffer, as
            //for example when display2d:true, the ouputPrompt doesn't
            //need to be in the first line
            if(m_outputCache.isEmpty())
            {
                m_outputCache.prepend(m_errCache);
                m_errCache.clear();
            }
            m_outputCache+=line+'\n';
            m_onStdoutStroke=true;
            couldBeQuestion=false;
        }else
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
    m_outputCache.clear();
    m_onStdoutStroke=false;
    m_errCache.clear();
}

void MaximaExpression::parseTexResult(const QString& text)
{
    const QString& output=text.trimmed();

    m_outputCache+=output;

    kDebug()<<"parsing "<<text;
    if(m_outputCache.contains(MaximaSession::MaximaPrompt))
    {
        kDebug()<<"got prompt";
        QString latex=m_outputCache.mid(0, m_outputCache.indexOf(MaximaSession::MaximaOutputPrompt)).trimmed();
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

        kDebug()<<"latex: "<<latex;
        MathematiK::TextResult* result=new MathematiK::TextResult(latex);
        result->setFormat(MathematiK::TextResult::LatexFormat);

        m_outputCache.clear();
        setResult(result);
        setStatus(MathematiK::Expression::Done);
    }
}

void MaximaExpression::evalFinished()
{
    kDebug()<<"evaluation finished";
    kDebug()<<"out: "<<m_outputCache;
    kDebug()<<"err: "<<m_errCache;

    QString text=m_outputCache;
    if(!session()->isTypesettingEnabled()) //don't screw up Maximas Ascii-Art
        text.replace(' ', "&nbsp;");
    MathematiK::TextResult* result;

    if(m_isHelpRequest)
    {
        result=new MathematiK::HelpResult(m_errCache);
        setResult(result);
        setStatus(MathematiK::Expression::Done);
    }
    else
    {
        result=new MathematiK::TextResult(text);

        setResult(result);

        if(!m_errCache.isEmpty())
        {
            setErrorMessage(m_errCache);
            setStatus(MathematiK::Expression::Error);
        }
        else
        {
            setStatus(MathematiK::Expression::Done);
        }
    }

    m_outputCache.clear();
    m_errCache.clear();
}

bool MaximaExpression::needsLatexResult()
{
    bool needsLatex=session()->isTypesettingEnabled() &&
        status()!=MathematiK::Expression::Error &&
        finishingBehavior()==MathematiK::Expression::DoNotDelete;

    if (result()&&result()->type()==MathematiK::TextResult::Type&&result()->data().toString()!="false" )
       return needsLatex && dynamic_cast<MathematiK::TextResult*>(result())->format()!=MathematiK::TextResult::LatexFormat;
    else
        return false;
}

void MaximaExpression::imageChanged()
{
    kDebug()<<"the temp image has changed";
    setResult( new MathematiK::EpsResult( KUrl(m_tempFile->fileName()) ) );
    setStatus(MathematiK::Expression::Done);
}

#include "maximaexpression.moc"
