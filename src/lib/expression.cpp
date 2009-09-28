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
using namespace Cantor;

#include "config-cantorlib.h"

#include "session.h"
#include "result.h"
#include "textresult.h"
#include "imageresult.h"
#include "latexresult.h"
#include "settings.h"

#include <QFileInfo>

#include <kstandarddirs.h>
#include <kprocess.h>
#include <ktemporaryfile.h>
#include <kdebug.h>
#include <kzip.h>


class Cantor::ExpressionPrivate
{
public:
    ExpressionPrivate() {
        result=0;
        session=0;
    }

    int id;
    QString command;
    QString error;
    QList<QString> information;
    Result* result;
    Expression::Status status;
    Session* session;
    Expression::FinishingBehavior finishingBehavior;

    QString latexFilename;
};

static const QString tex="\\documentclass[12pt]{article}                \n "\
                         "\\usepackage{latexsym,amsfonts,amssymb,ulem}  \n "\
                         "\\usepackage[dvips]{graphicx}                 \n "\
                         "\\setlength\\textwidth{5in}                   \n "\
                         "%1                                            \n "\
                         "\\pagestyle{empty}                            \n "\
                         "\\begin{document}                             \n "\
                         "%2                                            \n "\
                         "\\end{document}\n";


Expression::Expression( Session* session ) : QObject( session ),
                                             d(new ExpressionPrivate)
{
    d->session=session;
    d->id=session->nextExpressionId();
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

    if(result!=0)
    {
        kDebug()<<"settting result to a type "<<result->type()<<" result";
        #ifdef WITH_EPS
        //If it's text, and latex typesetting is enabled, render it
        if ( session()->isTypesettingEnabled()&&
             result->type()==TextResult::Type &&
             dynamic_cast<TextResult*>(result)->format()==TextResult::LatexFormat &&
             !result->toHtml().trimmed().isEmpty()
            )
        {
            renderResultAsLatex();
        }
        #endif
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

    if(status==Expression::Done&&d->finishingBehavior==Expression::DeleteOnFinish)
        deleteLater();
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

    QString dir=KGlobal::dirs()->saveLocation("tmp", "cantor/");

    //Check if the cantor subdir exists, if not, create it
    KTemporaryFile *texFile=new KTemporaryFile();
    texFile->setPrefix( "cantor/" );
    texFile->setSuffix( ".tex" );
    texFile->open();

    QString expressionTex=tex;
    expressionTex=expressionTex.arg(additionalLatexHeaders());
    expressionTex=expressionTex.arg(result()->data().toString().trimmed());

    texFile->write(expressionTex.toUtf8());
    texFile->flush();

    QString fileName = texFile->fileName();
    kDebug()<<"fileName: "<<fileName;
    d->latexFilename=fileName;
    d->latexFilename.replace(".tex", ".eps");
    KProcess *p=new KProcess( this );
    p->setWorkingDirectory(dir);

    (*p)<<Settings::self()->latexCommand()<<"-interaction=batchmode"<<"-halt-on-error"<<fileName;

    connect(p, SIGNAL( finished(int,  QProcess::ExitStatus) ), this, SLOT( convertToPs() ) );
    p->start();
}

void Expression::convertToPs()
{
    kDebug()<<"converting to ps";
    QString dviFile=d->latexFilename;
    dviFile.replace(".eps", ".dvi");
    KProcess *p=new KProcess( this );
    kDebug()<<"running: "<<Settings::self()->dvipsCommand()<<"-E"<<"-o"<<d->latexFilename<<dviFile;
    (*p)<<Settings::self()->dvipsCommand()<<"-E"<<"-o"<<d->latexFilename<<dviFile;

    connect(p, SIGNAL( finished(int,  QProcess::ExitStatus) ), this, SLOT( latexRendered() ) );
    p->start();
}

void Expression::latexRendered()
{
    kDebug()<<"rendered file "<<d->latexFilename;
    //cleanup the temp directory a bit...
    QString dir=KGlobal::dirs()->saveLocation("tmp", "cantor/");
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
    if(QFileInfo(d->latexFilename).exists())
    {
        LatexResult* latex=new LatexResult(result()->data().toString().trimmed(), KUrl(d->latexFilename));
        setResult( latex );
    }
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

QString Expression::additionalLatexHeaders()
{
    return QString();
}

int Expression::id()
{
    return d->id;
}

void Expression::setId(int id)
{
    d->id=id;
    emit idChanged();
}

void Expression::setFinishingBehavior(Expression::FinishingBehavior behavior)
{
    d->finishingBehavior=behavior;
}

Expression::FinishingBehavior Expression::finishingBehavior()
{
    return d->finishingBehavior;
}

#include "expression.moc"

