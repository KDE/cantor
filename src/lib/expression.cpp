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

#include "expression.h"
using namespace MathematiK;

#include "session.h"
#include "result.h"
#include "textresult.h"
#include "imageresult.h"

#include <kstandarddirs.h>
#include <kprocess.h>
#include <ktemporaryfile.h>
#include <kdebug.h>
#include <kzip.h>


class MathematiK::ExpressionPrivate
{
public:
    ExpressionPrivate() {
        result=0;
        session=0;
    }

    QString command;
    QString error;
    Result* result;
    Expression::Status status;
    Session* session;

    QString latexConversionScript;
    QString latexFilename;
};

Expression::Expression( Session* session ) : QObject( session ),
                                             d(new ExpressionPrivate)
{
    d->session=session;
    if ( d->latexConversionScript.isNull() )
        d->latexConversionScript=KStandardDirs::findExe( "mathematik_latexconvert.sh" );
}

Expression::~Expression()
{
    delete d->result;
    delete d;
}

void Expression::setCommand(const QString& command)
{
    d->command=command;
}

QString Expression::command()
{
    return d->command;
}

void Expression::setErrorMessage(const QString& error)
{
    d->error=error;
}

QString Expression::errorMessage()
{
    return d->error;
}

void Expression::setResult(Result* result)
{
    if(d->result)
        delete d->result;

    d->result=result;

    kDebug()<<"settting result to a type "<<result->type()<<" result";
    //If it's text, and latex typesetting is enabled, render it
    if ( session()->isTypesettingEnabled()&&
         result->type()==TextResult::Type &&
         dynamic_cast<TextResult*>(result)->format()==TextResult::LatexFormat &&
         !result->toHtml().trimmed().isEmpty()
        )
    {
        renderResultAsLatex();
    }

    emit gotResult();
}

Result* Expression::result()
{
    return d->result;
}

void Expression::setStatus(Expression::Status status)
{
    d->status=status;
    emit statusChanged(status);
}

Expression::Status Expression::status()
{
    return d->status;
}

Session* Expression::session()
{
    return d->session;
}
void Expression::renderResultAsLatex()
{
    kDebug()<<"rendering as latex";
    kDebug()<<"checking if it really is a formula that can be typeset";

    //Code taken from Kopete Latex plugin
    KTemporaryFile *tempFile=new KTemporaryFile();
    tempFile->setPrefix( "mathematik_latex-" );
    tempFile->setSuffix( ".png" );
    tempFile->open();

    QString fileName = tempFile->fileName();

    KProcess *p=new KProcess( this );

    QString argumentRes = QString( "-r %1x%2" ).arg( 120 ).arg( 120 );//LatexConfig::horizontalDPI() ).arg( LatexConfig::verticalDPI() );
    QString argumentOut = QString( "-o %1" ).arg( fileName );
    QString argumentInclude ( "-x %1" );
    //QString argumentFormat = "-fgif";  //we uses gif format because MSN only handle gif
    //LatexConfig::self()->readConfig();
    //QString includePath = LatexConfig::latexIncludeFile();
    // if ( !includePath.isNull() )
    //    p << m_convScript <<  argumentRes << argumentOut /*<< argumentFormat*/ << argumentInclude.arg( includePath ) << latexFormula;
    //else

    (*p) << d->latexConversionScript <<  argumentRes << argumentOut /*<< argumentFormat*/ << result()->data().toString();

    kDebug() << "Rendering" << d->latexConversionScript << argumentRes << argumentOut << result()->data().toString();

    d->latexFilename=fileName;

    connect(p, SIGNAL( finished(int,  QProcess::ExitStatus) ), this, SLOT( latexRendered() ) );
    p->start();
}

void Expression::latexRendered()
{
    kDebug()<<"rendered file "<<d->latexFilename;
    //replace the textresult with the rendered latex image result
    ImageResult* latex=new ImageResult( d->latexFilename , result()->data().toString() );
    setResult( latex );
}

//saving code
QDomElement Expression::toXml(QDomDocument& doc)
{
    QDomElement expr=doc.createElement( "Expression" );
    QDomElement cmd=doc.createElement( "Command" );
    QDomText cmdText=doc.createTextNode( command() );
    cmd.appendChild( cmdText );
    expr.appendChild( cmd );
    if ( result() )
    {
        kDebug()<<"result: "<<result();
        QDomElement resXml=result()->toXml( doc );
        expr.appendChild( resXml );
    }

    return expr;
}

void Expression::saveAdditionalData(KZip* archive)
{
    //just pass this call to the result
    if(result())
        result()->saveAdditionalData(archive);
}

#include "expression.moc"



