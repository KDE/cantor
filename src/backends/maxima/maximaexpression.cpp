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

    ///if(output.contains("maxima: "));
    //output=output.mid(6).trimmed();

    m_outputCache+=output+'\n';

    if(output.contains(MaximaSession::MaximaPrompt))
    {
        m_outputCache.remove(MaximaSession::MaximaPrompt);
        m_outputCache.remove(MaximaSession::MaximaOutputPrompt);
        evalFinished();
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

    if ( m_imagePath.isNull() ) //If this result contains a file, drop the text information
    {
        //strip html formatting
        QString stripped=m_outputCache;
        stripped.remove( QRegExp( "<[a-zA-Z\\/][^>]*>" ) );
        if (stripped.endsWith('\n'))
            stripped.chop(1);

        if (m_isContextHelpRequest)
        {
            QStringList l=stripped.trimmed().split("', '");
            l[0]=l[0].mid(2);
            l.last().chop(2);
            kDebug()<<"list: "<<l;
            setResult(new MathematiK::ContextHelpResult(l));
        }
        else
        {
            MathematiK::TextResult* result=0;
            if (m_isHelpRequest)
            {
                result=new MathematiK::HelpResult(stripped);
            }
            else
            {
                result=new MathematiK::TextResult(stripped);
            }

            if(m_outputCache.contains("class=\"math\"")) //It's latex stuff
                result->setFormat(MathematiK::TextResult::LatexFormat);

            setResult(result);
        }
    }
    else
    {
      setResult( new MathematiK::ImageResult( KUrl(m_imagePath ),i18n("Result of %1" ).arg( command() ) ) );
    }
    setStatus(MathematiK::Expression::Done);
}


#include "maximaexpression.moc"
