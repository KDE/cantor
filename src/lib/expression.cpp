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

#include <config-cantorlib.h>

#include "session.h"
#include "result.h"
#include "textresult.h"
#include "imageresult.h"
#include "latexresult.h"
#include "settings.h"
#include "latexrenderer.h"

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
    QList<Result*> results;
    QList<int> latexResultIndices;
    Expression::Status status;
    Session* session;
    Expression::FinishingBehavior finishingBehavior;
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
    QList<Result*> results;
    if (result)
        results.append(result);
    setResults(results)
}

void Expression::setResults(QList<Result*> results)
{
    foreach(Result* r, d->results) {
        delete r;
    }
    r->results.clear();

    d->results = results;

    d->latexResultIndices.clear();
    for(int i = 0; i < d->results.size(); ++i) {
        Result* r = d->results.at(i);
        kDebug()<<"setting result to a type "<<r->type()<<" result";
        #ifdef WITH_EPS
        //If it's text, and latex typesetting is enabled, render it
        if ( session()->isTypesettingEnabled()&&
             r->type()==TextResult::Type &&
             dynamic_cast<TextResult*>(r)->format()==TextResult::LatexFormat &&
             !r->toHtml().trimmed().isEmpty()
            )
        {
            d->latexResultIndices.append(i);
        }
        #endif
    }

    if (!d->latexResultIndices.isEmpty())
        renderResultsAsLatex();

    emit gotResult();
}

QList<Result*> Expression::results()
{
    return d->results;
}

void Expression::clearResults()
{
    foreach(Result* r, d->results) {
        delete d;
    }

    r->results.clear();
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

    int i = d->latexResultIndices.first();
    Result* result = d->results.at(i);
    LatexRenderer* renderer=new LatexRenderer(this);
    renderer->setLatexCode(result->data().toString().trimmed());
    renderer->addHeader(additionalLatexHeaders());

    connect(renderer, SIGNAL(done()), this, SLOT(latexRendered()));
    connect(renderer, SIGNAL(error()), this, SLOT(latexRendered()));

    renderer->render();
}

void Expression::latexRendered()
{
    LatexRenderer* renderer=qobject_cast<LatexRenderer*>(sender());

    int i = d->latexResultIndices.first();
    Result* result = d->results.at(i);
    kDebug()<<"rendered a result to "<<renderer->imagePath();
    //replace the textresult with the rendered latex image result
    //ImageResult* latex=new ImageResult( d->latexFilename );
    if(renderer->renderingSuccessful())
    {
        LatexResult* latex=new LatexResult(result()->data().toString().trimmed(), KUrl(renderer->imagePath()));
        delete d->results.at(i);
        d->results[i] = latex;
    } else
    {
        kDebug()<<"error rendering latex: "<<renderer->errorMessage();
    }

    d->latexResultIndices.removeFirst();
    if (!d->latexResultIndices.isEmpty()) {
        renderResultAsLatex();
    }

    renderer->deleteLater();
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

