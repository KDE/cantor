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
#include "epsresult.h"

#include <QDir>
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
    QList<QString> information;
    Result* result;
    Expression::Status status;
    Session* session;

    QString latexCmd;
    QString dviPsCmd;
    QString latexFilename;
};

static const QString tex="\\documentclass[12pt]{article}                \n \
                          \\usepackage{latexsym,amsfonts,amssymb,ulem}  \n \
                          \\usepackage[dvips]{graphicx}                 \n \
                          \\pagestyle{empty}                            \n \
                          \\begin{document}                             \n \
                          \\begin{eqnarray*}                            \n \
                          %1                                            \n \
                          \\end{eqnarray*}                              \n \
                          \\end{document}\n";


Expression::Expression( Session* session ) : QObject( session ),
                                             d(new ExpressionPrivate)
{
    d->session=session;
    d->latexCmd=KStandardDirs::findExe( "latex" );
    d->dviPsCmd=KStandardDirs::findExe( "dvips" );
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

    QString dir=KGlobal::dirs()->saveLocation("tmp", "mathematik/");

    //Check if the mathematik subdir exists, if not, create it
    KTemporaryFile *texFile=new KTemporaryFile();
    texFile->setPrefix( "mathematik/" );
    texFile->setSuffix( ".tex" );
    texFile->open();

    texFile->write(tex.arg(result()->data().toString().trimmed()).toUtf8());
    texFile->flush();

    QString fileName = texFile->fileName();
    kDebug()<<"fileName: "<<fileName;
    d->latexFilename=fileName;
    d->latexFilename.replace(".tex", ".eps");
    KProcess *p=new KProcess( this );
    p->setWorkingDirectory(dir);

    (*p)<<d->latexCmd<<"-interaction=batchmode"<<"-halt-on-error"<<fileName;

    connect(p, SIGNAL( finished(int,  QProcess::ExitStatus) ), this, SLOT( convertToPs() ) );
    p->start();
}

void Expression::convertToPs()
{
    kDebug()<<"converting to ps";
    QString dviFile=d->latexFilename;
    dviFile.replace(".eps", ".dvi");
    KProcess *p=new KProcess( this );
    kDebug()<<"running: "<<d->dviPsCmd<<"-E"<<"-o"<<d->latexFilename<<dviFile;
    (*p)<<d->dviPsCmd<<"-E"<<"-o"<<d->latexFilename<<dviFile;

    connect(p, SIGNAL( finished(int,  QProcess::ExitStatus) ), this, SLOT( latexRendered() ) );
    p->start();
}

void Expression::latexRendered()
{
    kDebug()<<"rendered file "<<d->latexFilename;
    //cleanup the temp directory a bit...
    QString dir=KGlobal::dirs()->saveLocation("tmp", "mathematik/");
    QStringList unneededExtensions;
    unneededExtensions<<".log"<<".aux"<<".tex"<<".dvi";
    foreach(const QString& ext, unneededExtensions)
    {
        QString s=d->latexFilename;
        s.replace(".eps", ext);
        QFile f(s);
        //f.remove();
    }

    //replace the textresult with the rendered latex image result
    //ImageResult* latex=new ImageResult( d->latexFilename );
    EpsResult* latex=new EpsResult(KUrl(d->latexFilename));
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

void Expression::addInformation(const QString& information)
{
    d->information.append(information);
}

#include "expression.moc"



