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

#include "config.h"

#include "lib/result.h"
#include "lib/epsresult.h"
#include "lib/latexresult.h"

#ifdef LIBSPECTRE_FOUND
  #include "libspectre/spectre.h"
#endif

#include <kdebug.h>
#include <klocale.h>
#include <QTextDocument>

ResultProxy::ResultProxy(QTextDocument* parent) : QObject(parent)
{
    m_document=parent;
    m_scale=1.0;
}

ResultProxy::~ResultProxy()
{

}

void ResultProxy::setScale(qreal scale)
{
    m_scale=scale;
}

void ResultProxy::scale(qreal value)
{
    m_scale*=value;
}

void ResultProxy::insertResult(QTextCursor& cursor, Cantor::Result* result)
{
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
            format=renderEps(result);
            if(format.isValid())
                cursor.insertText(QString(QChar::ObjectReplacementCharacter),  renderEps(result) );
            else
                cursor.insertText(i18n("Can not render Eps file. You may need additional packages"));

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
QTextCharFormat ResultProxy::renderEps(Cantor::Result* result)
{
#ifdef LIBSPECTRE_FOUND
    QTextImageFormat epsCharFormat;

    SpectreDocument* doc=spectre_document_new();;
    SpectreRenderContext* rc=spectre_render_context_new();

    KUrl url=result->data().toUrl();
    kDebug()<<"rendering eps file: "<<url;

    spectre_document_load(doc, url.toLocalFile().toUtf8());

    int w, h;
    double scale=1.8*m_scale;
    spectre_document_get_page_size(doc, &w, &h);
    kDebug()<<"dimension: "<<w<<"x"<<h;
    unsigned char* data;
    int rowLength;

    spectre_render_context_set_scale(rc, scale, scale);
    spectre_document_render_full( doc, rc, &data, &rowLength);

    QImage img(data, w*scale, h*scale, rowLength, QImage::Format_RGB32);

    m_document->addResource(QTextDocument::ImageResource, url, QVariant(img) );
    epsCharFormat.setName(url.url());

    spectre_document_free(doc);
    spectre_render_context_free(rc);

    return epsCharFormat;
#else
    Q_UNUSED(result);
    return QTextFormat().toCharFormat();
#endif

}
