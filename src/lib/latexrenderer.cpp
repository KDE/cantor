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

#include <QProcess>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QEventLoop>
#include <QTemporaryFile>
#include <KColorScheme>
#include <QUuid>
#include <QApplication>

#include <config-cantorlib.h>
#include "settings.h"

class Cantor::LatexRendererPrivate
{
  public:
    QString latexCode;
    QString header;
    LatexRenderer::Method method;
    bool isEquationOnly;
    LatexRenderer::EquationType equationType;
    QString errorMessage;
    bool success;
    QString latexFilename;
    QString epsFilename;
    QString uuid;
    QTemporaryFile* texFile;
};

static const QLatin1String tex("\\documentclass[fleqn]{article}"\
                         "\\usepackage{latexsym,amsfonts,amssymb,ulem}"\
                         "\\usepackage{amsmath}"\
                         "\\usepackage[dvips]{graphicx}"\
                         "\\usepackage[utf8]{inputenc}"\
                         "\\usepackage{xcolor}"\
                         "\\setlength\\textwidth{5in}"\
                         "\\setlength{\\parindent}{0pt}"\
                         "%1"\
                         "\\pagecolor[rgb]{%2,%3,%4}"\
                         "\\pagestyle{empty}"\
                         "\\begin{document}"\
                         "\\color[rgb]{%5,%6,%7}"\
                         "\\fontsize{%8}{%8}\\selectfont\n"\
                         "%9\n"\
                         "\\end{document}");

static const QLatin1String eqnHeader("\\begin{eqnarray*}%1\\end{eqnarray*}");
static const QLatin1String inlineEqnHeader("$%1$");

LatexRenderer::LatexRenderer(QObject* parent) : QObject(parent),
                                                d(new LatexRendererPrivate)
{
    d->method=LatexMethod;
    d->isEquationOnly=false;
    d->equationType=InlineEquation;
    d->success=false;
    d->texFile=nullptr;
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

void LatexRenderer::setEquationType(LatexRenderer::EquationType type)
{
    d->equationType=type;
}

LatexRenderer::EquationType LatexRenderer::equationType() const
{
    return d->equationType;
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

void LatexRenderer::setEquationOnly(bool isEquationOnly)
{
    d->isEquationOnly=isEquationOnly;
}

bool LatexRenderer::isEquationOnly() const
{
    return d->isEquationOnly;
}


QString LatexRenderer::imagePath() const
{
    return d->epsFilename;
}

QString Cantor::LatexRenderer::uuid() const
{
    return d->uuid;
}

bool LatexRenderer::render()
{
    switch(d->method)
    {
        case LatexRenderer::LatexMethod:
            return renderWithLatex();

        case LatexRenderer::MmlMethod:
            return renderWithMml();

        default:
            return false;
    };
}

void LatexRenderer::renderBlocking()
{
    QEventLoop event;
    connect(this, &LatexRenderer::done, &event, &QEventLoop::quit);
    connect(this, &LatexRenderer::error, &event, &QEventLoop::quit);

    bool success = render();
    // We can't emit error before running event loop, so exit by passing false as an error indicator
    if (success)
        event.exec();
    else
        return;
}

bool LatexRenderer::renderWithLatex()
{
    qDebug()<<"rendering using latex method";
    QString dir=QStandardPaths::writableLocation(QStandardPaths::TempLocation);

    if (d->texFile)
        delete d->texFile;

    d->texFile=new QTemporaryFile(dir + QDir::separator() + QLatin1String("cantor_tex-XXXXXX.tex"));
    d->texFile->open();

    KColorScheme scheme(QPalette::Active);
    const QColor &backgroundColor=scheme.background().color();
    const QColor &foregroundColor=scheme.foreground().color();
    QString expressionTex=tex;
    expressionTex=expressionTex.arg(d->header)
                               .arg(backgroundColor.redF()).arg(backgroundColor.greenF()).arg(backgroundColor.blueF())
                               .arg(foregroundColor.redF()).arg(foregroundColor.greenF()).arg(foregroundColor.blueF());

    int fontPointSize = QApplication::font().pointSize();
    expressionTex=expressionTex.arg(fontPointSize);

    if(isEquationOnly())
    {
        switch(equationType())
        {
            case FullEquation: expressionTex=expressionTex.arg(eqnHeader); break;
            case InlineEquation: expressionTex=expressionTex.arg(inlineEqnHeader); break;
        }
    }
    expressionTex=expressionTex.arg(d->latexCode);

    // qDebug()<<"full tex:\n"<<expressionTex;

    d->texFile->write(expressionTex.toUtf8());
    d->texFile->flush();

    QString fileName = d->texFile->fileName();
    qDebug()<<"fileName: "<<fileName;
    d->latexFilename=fileName;
    QProcess *p=new QProcess( this );
    p->setWorkingDirectory(dir);

    d->uuid = genUuid();

    qDebug() << Settings::self()->latexCommand();
    QFileInfo info(Settings::self()->latexCommand());
    if (info.exists() && info.isExecutable())
    {
        p->setProgram(Settings::self()->latexCommand());
        p->setArguments({QStringLiteral("-jobname=cantor_") + d->uuid, QStringLiteral("-halt-on-error"), fileName});

        connect(p, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(convertToPs()) );
        p->start();
        return true;
    }
    else
    {
        setErrorMessage(QStringLiteral("failed to find latex executable"));
        return false;
    }
}

void LatexRenderer::convertToPs()
{
    const QString& dir=QStandardPaths::writableLocation(QStandardPaths::TempLocation);

    QString dviFile = dir + QDir::separator() + QStringLiteral("cantor_") + d->uuid + QStringLiteral(".dvi");
    d->epsFilename = dir + QDir::separator() + QLatin1String("cantor_")+d->uuid+QLatin1String(".eps");

    QProcess *p=new QProcess( this );
    qDebug()<<"converting to eps: "<<Settings::self()->dvipsCommand()<<"-E"<<"-o"<<d->epsFilename<<dviFile;

    QFileInfo info(Settings::self()->dvipsCommand());
    if (info.exists() && info.isExecutable())
    {
        p->setProgram(Settings::self()->dvipsCommand());
        p->setArguments({QStringLiteral("-E"), QStringLiteral("-q"), QStringLiteral("-o"), d->epsFilename, dviFile});

        connect(p, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(convertingDone()) );
        p->start();
    }
    else
    {
        setErrorMessage(QStringLiteral("failed to find dvips executable"));
        emit error();
    }
}

void LatexRenderer::convertingDone()
{
    QFileInfo info(d->epsFilename);
    qDebug() <<"remove temporary files for " << d->latexFilename;

    QString pathWithoutExtension = info.path() + QDir::separator() + info.completeBaseName();
    QFile::remove(pathWithoutExtension + QLatin1String(".log"));
    QFile::remove(pathWithoutExtension + QLatin1String(".aux"));
    QFile::remove(pathWithoutExtension + QLatin1String(".dvi"));

    if(info.exists())
    {
        delete d->texFile;
        d->texFile = nullptr;

        d->success=true;
        emit done();
    }
    else
    {
        d->success=false;
        setErrorMessage(QStringLiteral("failed to create the latex preview image"));
        emit error();
    }
}

bool LatexRenderer::renderWithMml()
{
    qWarning()<<"WARNING: MML rendering not implemented yet!";
    emit error();
    return false;
}

QString LatexRenderer::genUuid()
{
    QString uuid = QUuid::createUuid().toString();
    uuid.remove(0, 1);
    uuid.chop(1);
    uuid.replace(QLatin1Char('-'), QLatin1Char('_'));
    return uuid;
}
