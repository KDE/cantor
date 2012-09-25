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

#include "commandentry.h"
#include "textresultitem.h"
#include "lib/result.h"
#include "lib/textresult.h"
#include "lib/latexresult.h"

#include <QTextCursor>

#include <KFileDialog>
#include <KStandardAction>
#include <KAction>
#include <klocale.h>
#include <kdebug.h>

TextResultItem::TextResultItem(QGraphicsObject* parent)
    : WorksheetTextItem(parent), ResultItem()
{
    setTextInteractionFlags(Qt::TextSelectableByMouse);
    connect(this, SIGNAL(removeResult()), parentEntry(),
            SLOT(removeSendingResult()));
}

TextResultItem::~TextResultItem()
{
}

double TextResultItem::setGeometry(double x, double y, double w)
{
    return WorksheetTextItem::setGeometry(x, y, w);
}

void TextResultItem::populateMenu(KMenu* menu, const QPointF& pos)
{
    KAction* copy = KStandardAction::copy(this, SLOT(copy()), menu);
    if (!textCursor().hasSelection())
        copy->setEnabled(false);
    menu->addAction(copy);
    addCommonActions(this, menu);

    Cantor::Result* res = result();
    if (res->type() == Cantor::LatexResult::Type) {
        QAction* showCodeAction = 0;
        Cantor::LatexResult* lres = dynamic_cast<Cantor::LatexResult*>(res);
        if (lres->isCodeShown())
            showCodeAction = menu->addAction(i18n("Show Rendered"));
        else
            showCodeAction = menu->addAction(i18n("Show Code"));

        connect(showCodeAction, SIGNAL(triggered()), this,
                SLOT(toggleLatexCode()));
    }

    menu->addSeparator();
    kDebug() << "populate Menu";
    emit menuCreated(menu, mapToParent(pos));
}

void TextResultItem::update()
{
    switch(result()->type()) {
    case Cantor::TextResult::Type:
        {
            QTextCursor cursor = textCursor();
            cursor.movePosition(QTextCursor::Start);
            cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
            QString html = result()->toHtml();
            if (html.isEmpty())
                cursor.removeSelectedText();
            else
                cursor.insertHtml(html);
            break;
        }
    case Cantor::LatexResult::Type:
        setLatex(dynamic_cast<Cantor::LatexResult*>(result()));
        break;
    default:
        kDebug() << "ERROR: Invalid result type " << result()->type()
                 <<" in TextResultItem";
        break;
    }
}

void TextResultItem::setLatex(Cantor::LatexResult* result)
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    QString latex = result->toLatex().trimmed();
    if (latex.startsWith("\\begin{eqnarray*}") &&
        latex.endsWith("\\end{eqnarray*}")) {
        latex = latex.mid(17);
        latex = latex.left(latex.size() - 15);
    }
    if (result->isCodeShown()) {
        if (latex.isEmpty())
            cursor.removeSelectedText();
        else
            cursor.insertText(latex);
    } else {
        QTextImageFormat format;
        format = epsRenderer()->render(cursor.document(),
                                       result->data().toUrl());
        format.setProperty(EpsRenderer::CantorFormula,
                           EpsRenderer::LatexFormula);
        format.setProperty(EpsRenderer::Code, latex);
        format.setProperty(EpsRenderer::Delimiter, "$$");
        if(format.isValid())
            cursor.insertText(QString(QChar::ObjectReplacementCharacter), format);
        else
            cursor.insertText(i18n("Cannot render Eps file. You may need additional packages"));
    }
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

void TextResultItem::saveResult()
{
    Cantor::Result* res = result();
    const QString& filename = KFileDialog::getSaveFileName(KUrl(), res->mimeType(), worksheet()->worksheetView());
    kDebug() << "saving result to " << filename;
    res->save(filename);
}

void TextResultItem::deleteLater()
{
    WorksheetTextItem::deleteLater();
}

EpsRenderer* TextResultItem::epsRenderer()
{
    return qobject_cast<Worksheet*>(scene())->epsRenderer();
}

CommandEntry* TextResultItem::parentEntry()
{
    return qobject_cast<CommandEntry*>(parentObject());
}

#include "textresultitem.moc"

