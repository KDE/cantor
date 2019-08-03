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
    Copyright (C) 2012 Martin Kuettler <martin.kuettler@gmail.com>
 */

#include "textresultitem.h"
#include "commandentry.h"
#include "lib/result.h"
#include "lib/textresult.h"
#include "lib/latexresult.h"
#include "lib/epsrenderer.h"
#include "lib/mimeresult.h"
#include "lib/htmlresult.h"
#include "mathrendertask.h"
#include "config-cantor.h"

#include <QDebug>
#include <QFileDialog>
#include <QTextCursor>

#include <KStandardAction>
#include <KLocalizedString>

TextResultItem::TextResultItem(QGraphicsObject* parent, Cantor::Result* result)
    : WorksheetTextItem(parent), ResultItem(result)
{
    setTextInteractionFlags(Qt::TextSelectableByMouse);
    update();
}

double TextResultItem::setGeometry(double x, double y, double w)
{
    return WorksheetTextItem::setGeometry(x, y, w);
}

void TextResultItem::populateMenu(QMenu* menu, QPointF pos)
{
    QAction * copy = KStandardAction::copy(this, SLOT(copy()), menu);
    if (!textCursor().hasSelection())
        copy->setEnabled(false);
    menu->addAction(copy);
    ResultItem::addCommonActions(this, menu);

    Cantor::Result* res = result();
    if (res->type() == Cantor::LatexResult::Type) {
        QAction* showCodeAction = nullptr;
        Cantor::LatexResult* lres = dynamic_cast<Cantor::LatexResult*>(res);
        if (lres->isCodeShown())
            showCodeAction = menu->addAction(i18n("Show Rendered"));
        else
            showCodeAction = menu->addAction(i18n("Show Code"));

        connect(showCodeAction, &QAction::triggered, this, &TextResultItem::toggleLatexCode);
    } else if (res->type() == Cantor::HtmlResult::Type) {
        Cantor::HtmlResult* hres = static_cast<Cantor::HtmlResult*>(res);
        switch (hres->format())
        {
            case Cantor::HtmlResult::Html:
                connect(menu->addAction(i18n("Show Html Code")), &QAction::triggered, this, &TextResultItem::showHtmlSource);
                if (!hres->plain().isEmpty())
                    connect(menu->addAction(i18n("Show Plain Alternative")), &QAction::triggered, this, &TextResultItem::showPlain);
                break;

            case Cantor::HtmlResult::HtmlSource:
                connect(menu->addAction(i18n("Show Html")), &QAction::triggered, this, &TextResultItem::showHtml);
                if (!hres->plain().isEmpty())
                    connect(menu->addAction(i18n("Show Plain Alternative")), &QAction::triggered, this, &TextResultItem::showPlain);
                break;

            case Cantor::HtmlResult::PlainAlternative:
                connect(menu->addAction(i18n("Show Html")), &QAction::triggered, this, &TextResultItem::showHtml);
                connect(menu->addAction(i18n("Show Html Code")), &QAction::triggered, this, &TextResultItem::showHtmlSource);
                break;

        }
    }

    menu->addSeparator();
    qDebug() << "populate Menu";
    emit menuCreated(menu, mapToParent(pos));
}

void TextResultItem::update()
{
    Q_ASSERT(
        m_result->type() == Cantor::TextResult::Type
        || m_result->type() == Cantor::LatexResult::Type
        || m_result->type() == Cantor::MimeResult::Type
        || m_result->type() == Cantor::HtmlResult::Type
    );
    switch(m_result->type()) {
    case Cantor::TextResult::Type:
    case Cantor::MimeResult::Type:
    case Cantor::HtmlResult::Type:
        setHtml(m_result->toHtml());
        break;
    case Cantor::LatexResult::Type:
        setLatex(dynamic_cast<Cantor::LatexResult*>(m_result));
        break;
    default:
        break;
    }
}

void TextResultItem::setLatex(Cantor::LatexResult* result)
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    QString latex = result->toLatex().trimmed();
    if (latex.startsWith(QLatin1String("\\begin{eqnarray*}")) &&
        latex.endsWith(QLatin1String("\\end{eqnarray*}"))) {
        latex = latex.mid(17);
        latex = latex.left(latex.size() - 15);
    }

#ifdef WITH_EPS
    if (result->isCodeShown()) {
        if (latex.isEmpty())
            cursor.removeSelectedText();
        else
            cursor.insertText(latex);
    } else {
        QTextImageFormat format;

        if (!result->image().isNull() && worksheet()->epsRenderer()->scale() == 1.0)
        {
            cursor.insertText(QString(QChar::ObjectReplacementCharacter), toFormat(result->image(), latex));
        }
        else
        {
            Cantor::EpsRenderer* renderer = qobject_cast<Worksheet*>(scene())->epsRenderer();;
            format = renderer->render(cursor.document(), result->url());
            format.setProperty(Cantor::EpsRenderer::CantorFormula,
                            Cantor::EpsRenderer::LatexFormula);
            format.setProperty(Cantor::EpsRenderer::Code, latex);
            format.setProperty(Cantor::EpsRenderer::Delimiter, QLatin1String("$$"));
            if(format.isValid())
                cursor.insertText(QString(QChar::ObjectReplacementCharacter), format);
            else
                cursor.insertText(i18n("Cannot render Eps file. You may need additional packages"));
        }
    }
#else
    cursor.insertText(QString(QChar::ObjectReplacementCharacter), toFormat(result->image(), latex));
#endif
}

double TextResultItem::width() const
{
    return WorksheetTextItem::width();
}

double TextResultItem::height() const
{
    return WorksheetTextItem::height();
}

void TextResultItem::toggleLatexCode()
{
     Cantor::LatexResult* lr = dynamic_cast<Cantor::LatexResult*>(result());
     if(lr->isCodeShown())
         lr->showRendered();
     else
         lr->showCode();

     parentEntry()->updateEntry();
}

void TextResultItem::showHtml()
{
     Cantor::HtmlResult* hr = static_cast<Cantor::HtmlResult*>(result());
     hr->setFormat(Cantor::HtmlResult::Html);
     parentEntry()->updateEntry();
}

void TextResultItem::showHtmlSource()
{
     Cantor::HtmlResult* hr = static_cast<Cantor::HtmlResult*>(result());
     hr->setFormat(Cantor::HtmlResult::HtmlSource);
     parentEntry()->updateEntry();
}

void TextResultItem::showPlain()
{
     Cantor::HtmlResult* hr = static_cast<Cantor::HtmlResult*>(result());
     hr->setFormat(Cantor::HtmlResult::PlainAlternative);
     parentEntry()->updateEntry();
}

void TextResultItem::saveResult()
{
    Cantor::Result* res = result();
    const QString& filename = QFileDialog::getSaveFileName(worksheet()->worksheetView(), i18n("Save result"), QString(), res->mimeType());
    qDebug() << "saving result to " << filename;
    res->save(filename);
}

void TextResultItem::deleteLater()
{
    WorksheetTextItem::deleteLater();
}

QTextImageFormat TextResultItem::toFormat(const QImage& image, const QString& latex)
{
    QTextImageFormat format;

    QUrl internal;
    internal.setScheme(QLatin1String("internal"));
    internal.setPath(MathRenderTask::genUuid());

    document()->addResource(QTextDocument::ImageResource, internal, QVariant(image) );

    format.setName(internal.url());
    format.setProperty(Cantor::EpsRenderer::CantorFormula, Cantor::EpsRenderer::LatexFormula);
    //format.setProperty(Cantor::EpsRenderer::ImagePath, filename);
    format.setProperty(Cantor::EpsRenderer::Code, latex);
    format.setProperty(Cantor::EpsRenderer::Delimiter, QLatin1String("$$"));

    return format;
}
