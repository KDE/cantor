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

#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>
#include <QTimer>
#include <QRegExp>

MaximaExpression::MaximaExpression( MathematiK::Session* session ) : MathematiK::Expression(session)
{
    kDebug();
}

MaximaExpression::~MaximaExpression()
{

}

void MaximaExpression::evaluate()
{
    kDebug()<<"evaluating "<<command();
    setStatus(MathematiK::Expression::Computing);
    m_imagePath=QString();

    m_aboutToReceiveLatex=false;
    m_isHelpRequest=false;
    m_isContextHelpRequest=false;
    //check if this is a ?command
    if(command().startsWith('?')||command().endsWith('?'))
        m_isHelpRequest=true;
    if(command().startsWith("dir("))
        m_isContextHelpRequest=true;

    dynamic_cast<MaximaSession*>(session())->appendExpressionToQueue(this);
}

void MaximaExpression::interrupt()
{
    kDebug()<<"interrupting";
    dynamic_cast<MaximaSession*>(session())->sendSignalToProcess(2);
}

void MaximaExpression::parseOutput(const QString& text)
{
    QString output=text.trimmed();
    kDebug()<<"parsing: "<<output;

    ///if(output.contains("maxima: "));
    //output=output.mid(6).trimmed();

    m_outputCache+=output+'\n';

    if(output.contains(MaximaSession::MaximaPrompt))
    {
        kDebug()<<"got prompt";
        m_outputCache.remove(MaximaSession::MaximaPrompt);
        m_outputCache.remove(MaximaSession::MaximaOutputPrompt);
        evalFinished();
    }

    //Check if it's a question from maxima
    if(m_outputCache.contains(QRegExp("Is(.*)zero or nonzero.*")) || m_outputCache.contains(QRegExp("Is(.*)positive, negative, or z")))
    {
        kDebug()<<"Maxima asked a question of zero/nonzero";
        qobject_cast<MaximaSession*>(session())->sendInputToProcess(";\n");//Abort the question
        setResult(new MathematiK::TextResult(i18n("Maxima asked \"%1\"", m_outputCache.trimmed())));
        setStatus(MathematiK::Expression::Error);
    }
}

void MaximaExpression::parseError(const QString& text)
{
    kDebug()<<"error";
    setResult(new MathematiK::TextResult(text));
    setStatus(MathematiK::Expression::Error);
}

void MaximaExpression::addFileResult( const QString& path )
{
  KUrl url( path );
  if ( url.fileName().endsWith(".png") )
  {
    kDebug()<<"adding file "<<path<<"   "<<url;
    m_imagePath=path;
  }
}

void MaximaExpression::evalFinished()
{
    kDebug()<<"evaluation finished";
    kDebug()<<m_outputCache;

    QString text=m_outputCache;
    bool isLatex=false;
    if (text.startsWith("$$"))
    {
        text=text.mid(2, text.indexOf("$$", 3)-2);
        isLatex=true;
    }

    MathematiK::TextResult* result=new MathematiK::TextResult(text);
    if (isLatex)
        result->setFormat(MathematiK::TextResult::LatexFormat);

    setResult(result);
    setStatus(MathematiK::Expression::Done);
}

bool MaximaExpression::needsLatexResult()
{
    return session()->isTypesettingEnabled() && !m_aboutToReceiveLatex && status()!=MathematiK::Expression::Error;
}

void MaximaExpression::aboutToReceiveLatex()
{
    m_aboutToReceiveLatex=true;
    m_outputCache=QString();
}


#include "maximaexpression.moc"
