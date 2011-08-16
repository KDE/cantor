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
    Copyright (C) 2011 Alexander Rieder <alexanderrieder@gmail.com>
 */

#include "latexrenderer.h"
using namespace Cantor;

#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <kprocess.h>
#include <kdebug.h>
#include <QFileInfo>

#include <config-cantorlib.h>
#include "settings.h"

class Cantor::LatexRendererPrivate
{
  public:
    QString latexCode;
    QString header;
    LatexRenderer::Method method;
    QString errorMessage;
    bool success;
    QString latexFilename;
};

static const QString tex="\\documentclass[12pt,fleqn]{article}          \n "\
                         "\\usepackage{latexsym,amsfonts,amssymb,ulem}  \n "\
                         "\\usepackage[dvips]{graphicx}                 \n "\
                         "\\setlength\\textwidth{5in}                   \n "\
                         "\\setlength{\\parindent}{0pt}                 \n "\
                         "%1                                            \n "\
                         "\\pagestyle{empty}                            \n "\
                         "\\begin{document}                             \n "\
                         "%2                                            \n "\
                         "\\end{document}\n";


LatexRenderer::LatexRenderer(QObject* parent) : QObject(parent),
                                                d(new LatexRendererPrivate)
{
    d->method=LatexMethod;
    d->success=false;
}

LatexRenderer::~LatexRenderer()
{
    delete d;
}

QString LatexRenderer::latexCode() const
{
    return d->latexCode;
}

void LatexRenderer::setLatexCode(const QString& src)
{
    d->latexCode=src;
}

QString LatexRenderer::header() const
{
    return d->header;
}

void LatexRenderer::addHeader(const QString& header)
{
    d->header.append(header);
}

void LatexRenderer::setHeader(const QString& header)
{
    d->header=header;
}

LatexRenderer::Method LatexRenderer::method() const
{
    return d->method;
}

void LatexRenderer::setMethod(LatexRenderer::Method method)
{
    d->method=method;
}

void LatexRenderer::setErrorMessage(const QString& msg)
{
    d->errorMessage=msg;
}

QString LatexRenderer::errorMessage() const
{
    return d->errorMessage;
}

bool LatexRenderer::renderingSuccessful() const
{
    return d->success;
}

QString LatexRenderer::imagePath() const
{
    return d->latexFilename;
}

void LatexRenderer::render()
{
    switch(d->method)
    {
        case LatexRenderer::LatexMethod:  renderWithLatex(); break;
        case LatexRenderer::MmlMethod:    renderWithMml(); break;
    };
}

void LatexRenderer::renderWithLatex()
{
    kDebug()<<"rendering using latex method";
    QString dir=KGlobal::dirs()->saveLocation("tmp", "cantor/");

    //Check if the cantor subdir exists, if not, create it
    KTemporaryFile *texFile=new KTemporaryFile();
    texFile->setPrefix( "cantor/" );
    texFile->setSuffix( ".tex" );
    texFile->open();

    QString expressionTex=tex;
    expressionTex=expressionTex.arg(d->header);
    expressionTex=expressionTex.arg(d->latexCode);

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

void LatexRenderer::convertToPs()
{
    kDebug()<<"converting to ps";
    QString dviFile=d->latexFilename;
    dviFile.replace(".eps", ".dvi");
    KProcess *p=new KProcess( this );
    kDebug()<<"running: "<<Settings::self()->dvipsCommand()<<"-E"<<"-o"<<d->latexFilename<<dviFile;
    (*p)<<Settings::self()->dvipsCommand()<<"-E"<<"-o"<<d->latexFilename<<dviFile;

    connect(p, SIGNAL( finished(int,  QProcess::ExitStatus) ), this, SLOT( convertingDone() ) );
    p->start();
}

void LatexRenderer::convertingDone()
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

    if(QFileInfo(d->latexFilename).exists())
    {
        d->success=true;
        emit done();
    }else
    {
        d->success=false;
        setErrorMessage("something is wrong");
        emit error();
    }
}

void LatexRenderer::renderWithMml()
{
    kDebug()<<"WARNING: MML rendering not implemented yet!";
    emit done();
}

#include "latexrenderer.moc"
