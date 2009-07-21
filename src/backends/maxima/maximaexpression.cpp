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
    m_isContextHelpRequest=false;
    if(m_tempFile)
        m_tempFile->deleteLater();
    m_tempFile=0;
    //check if this is a ?command
    if(command().startsWith('?')||command().startsWith("describe(")||command().startsWith("example("))
        m_isHelpRequest=true;

    m_onStdoutStroke=false;
    m_outputCache.clear();
    m_errCache.clear();

    if(command().startsWith("plot") && MaximaSettings::self()->integratePlots())
    {
        m_tempFile=new KTemporaryFile();
        m_tempFile->setPrefix( "mathematik_maxima-" );
        m_tempFile->setSuffix( ".png" );
        m_tempFile->open();

        QString fileName = m_tempFile->fileName();
        m_fileWatch.addFile(fileName);
        connect(&m_fileWatch, SIGNAL(dirty(const QString&)), this, SLOT(imageChanged()));

        QString cmd=command();
        QString preamble="set terminal png size 500,340; set output '" + fileName + "';";
        QString plotParameters = "[gnuplot_preamble,\"" + preamble + "\"]";
        cmd.insert(cmd.lastIndexOf(')'), ","+plotParameters);

        setCommand(cmd);
    }

    dynamic_cast<MaximaSession*>(session())->appendExpressionToQueue(this);
}

void MaximaExpression::interrupt()
{
    kDebug()<<"interrupting";
    dynamic_cast<MaximaSession*>(session())->sendSignalToProcess(2);
}

void MaximaExpression::addInformation(const QString& information)
{
    kDebug()<<"adding information";
    QString inf=information;
    if(!inf.endsWith(';'))
        inf+=';';
    MathematiK::Expression::addInformation(inf);

    dynamic_cast<MaximaSession*>(session())->sendInputToProcess(inf+"\n");
}

void MaximaExpression::parseOutput(const QString& text)
{
    QString output=text.trimmed();
    kDebug()<<"parsing: "<<output;

    ///if(output.contains("maxima: "));
    //output=output.mid(6).trimmed();

    if(m_tempFile)
    {
        QTimer::singleShot(500, this, SLOT(imageChanged()));
        return;
    }

    QStringList lines=output.split('\n');
    foreach(QString line, lines)
    {
        if(line.endsWith('\r'))
            line.chop(1);
        if(line.isEmpty())
            continue;

        if (MaximaSession::MaximaPrompt.exactMatch(line))
        {
            evalFinished();
            m_onStdoutStroke=false;
            m_askTimer->stop();
        }else if(line.indexOf(MaximaSession::MaximaOutputPrompt)==0||m_onStdoutStroke)
        {
            line.remove(MaximaSession::MaximaOutputPrompt);
            m_outputCache+=line+'\n';
            m_onStdoutStroke=true;
            m_askTimer->stop();
        }else
        {
            kDebug()<<"got something";
            m_errCache+=line+'\n';
            m_onStdoutStroke=false;
            m_askTimer->stop();
            m_askTimer->start(ASK_TIME);
        }
    }

}

void MaximaExpression::askForInformation()
{
    emit needsAdditionalInformation(m_errCache.trimmed());
    m_outputCache.clear();
    m_onStdoutStroke=false;
    m_errCache.clear();
}

void MaximaExpression::parseError(const QString& text)
{
    kDebug()<<"error";
    setResult(new MathematiK::TextResult(text));
    setStatus(MathematiK::Expression::Error);
}

void MaximaExpression::parseTexResult(const QString& text)
{
    QString output=text.trimmed();

    m_outputCache+=output;

    kDebug()<<"parsing "<<text;
    if(m_outputCache.contains(MaximaSession::MaximaPrompt))
    {
        kDebug()<<"got prompt";
        QString latex=m_outputCache.mid(0, m_outputCache.indexOf(MaximaSession::MaximaOutputPrompt)).trimmed();
        if(latex.startsWith("$$"))
        {
            latex=latex.mid(2);
            latex.prepend("\\begin{eqnarray*}");
        }
        if(latex.endsWith("$$"))
        {
            latex.chop(2);
            latex.append("\\end{eqnarray*}");
        }

        kDebug()<<"latex: "<<latex;
        MathematiK::TextResult* result=new MathematiK::TextResult(latex);
        result->setFormat(MathematiK::TextResult::LatexFormat);

        m_outputCache=QString();
        setResult(result);
        setStatus(MathematiK::Expression::Done);
    }
}

void MaximaExpression::evalFinished()
{
    kDebug()<<"evaluation finished";
    kDebug()<<"out: "<<m_outputCache;
    kDebug()<<"err: "<<m_errCache;

    QString text=m_outputCache.trimmed();
    MathematiK::TextResult* result;

    if(m_isHelpRequest)
    {
        result=new MathematiK::HelpResult(m_errCache.trimmed());
        setResult(result);
        setStatus(MathematiK::Expression::Done);
    }
    else
    {
        result=new MathematiK::TextResult(text);

        setResult(result);
        setStatus(MathematiK::Expression::Done);

        if(!m_errCache.isEmpty())
        {
            setErrorMessage(m_errCache.trimmed());
            setStatus(MathematiK::Expression::Error);
        }
    }

    m_outputCache=QString();
    m_errCache=QString();
}

bool MaximaExpression::needsLatexResult()
{
    bool needsLatex=session()->isTypesettingEnabled() && status()!=MathematiK::Expression::Error;
    if (result()&&result()->type()==MathematiK::TextResult::Type&&result()->data().toString()!="false" )
       return needsLatex && dynamic_cast<MathematiK::TextResult*>(result())->format()!=MathematiK::TextResult::LatexFormat;
    else
        return false;
}

void MaximaExpression::imageChanged()
{
    kDebug()<<"the temp image has changed";
    setResult( new MathematiK::ImageResult( KUrl(m_tempFile->fileName()) ) );
    setStatus(MathematiK::Expression::Done);
}

#include "maximaexpression.moc"
