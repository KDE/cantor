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

#include "resultproxy.h"

#include <config-cantor.h>

#include "lib/result.h"
#include "lib/epsresult.h"
#include "lib/latexresult.h"
#include "lib/imageresult.h"
#include "lib/animationresult.h"
#include "animationhandler.h"
#include "animation.h"
#include "worksheet.h"

#include <kdebug.h>
#include <klocale.h>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QTextObjectInterface>
#include <QMovie>
#include <QTimer>

ResultProxy::ResultProxy(Worksheet* parent) : QObject(parent)
{
    m_worksheet = parent;
}

ResultProxy::~ResultProxy()
{

}

void ResultProxy::insertResult(QTextCursor& cursor, Cantor::Result* result)
{
    kDebug()<<"inserting new format";
    QTextCharFormat format;
    switch(result->type())
    {
        case Cantor::LatexResult::Type:
            //if the lr is in Code-Mode, insert its html.
            //otherwise fallthrough to the EpsRendering code
            if(dynamic_cast<Cantor::LatexResult*>(result)->isCodeShown())
            {
                QString html=result->toHtml().trimmed();
                if(html.isEmpty())
                    cursor.removeSelectedText();
                else
                    cursor.insertHtml(result->toHtml());

                break;
            }
        case Cantor::EpsResult::Type:
            format=renderEps(cursor.document(), result);
            if(format.isValid())
                cursor.insertText(QString(QChar::ObjectReplacementCharacter),  format );
            else
                cursor.insertText(i18n("Cannot render Eps file. You may need additional packages"));

            break;
        case Cantor::AnimationResult::Type:
            kDebug()<<"it's an animation";
            format=renderGif(result);
            if(format.isValid())
            {
                cursor.insertText(QString(QChar::ObjectReplacementCharacter), format );
                AnimationHelperItem movie = format.property(AnimationHandler::MovieProperty).value<AnimationHelperItem>();
                QTextCursor cursor2=cursor;
                cursor2.setPosition(cursor.position()-1);

                movie.setPosition(cursor2);
            }
            break;
        default:
            QString html=result->toHtml().trimmed();
            if(html.isEmpty())
                cursor.removeSelectedText();
            else
                cursor.insertHtml(result->toHtml());
    }
}

//private result specific rendering methods
QTextCharFormat ResultProxy::renderEps(QTextDocument* doc, Cantor::Result* result)
{
    QTextImageFormat format;
    format = m_worksheet->epsRenderer()->renderEps(doc, result->data().toUrl());
    if (result->type() == Cantor::LatexResult::Type) {
	kDebug() << "latex result" << result->toLatex();
	format.setProperty(EpsRenderer::CantorFormula, EpsRenderer::LatexFormula);
	//format.setProperty(EpsReader::ImagePath, "");
	QString latex = result->toLatex();
	if (latex.startsWith("\\begin{eqnarray*}"))
	    latex = latex.mid(17);
	if (latex.endsWith("\\end{eqnarray*}"))
	    latex = latex.left(latex.size() - 15);
	format.setProperty(EpsRenderer::Code, latex);
	format.setProperty(EpsRenderer::Delimiter, "$$");
    } else {
	kDebug() << "eps result";
    }
    return format;
}

QTextCharFormat ResultProxy::renderGif(Cantor::Result* result)
{
    QTextImageFormat charFormat;
    KUrl url=result->url();

    AnimationHelperItem anim;
    QMovie* movie=static_cast<QMovie*>(result->data().value<QObject*>());
    anim.setMovie(movie);

    charFormat.setProperty(AnimationHandler::MovieProperty, qVariantFromValue(anim));
    charFormat.setName(url.toLocalFile());

    charFormat.setName(url.url());

    QTimer::singleShot(0, movie, SLOT(start()));

    return charFormat;
}
