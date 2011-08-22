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
    Copyright (C) 2010 Raffaele De Feo <alberthilbert@gmail.com>
 */

#include "textentry.h"
#include "worksheetentry.h"
#include "worksheet.h"
#include "resultproxy.h"
#include "lib/defaulthighlighter.h"
#include "lib/latexrenderer.h"

#include "formulatextobject.h"

#include <QEvent>
#include <QKeyEvent>
#include <QTextDocumentFragment>
#include <QUrl>

#include <kdebug.h>
#include <kzip.h>
#include <kmenu.h>
#include <kicon.h>
#include <klocale.h>
#include <kstandardaction.h>
#include <kaction.h>


TextEntry::TextEntry(QTextCursor position, Worksheet* parent ) : WorksheetEntry( position, parent )
{
    QTextFrameFormat frameFormat = m_frame->frameFormat();
    frameFormat.setPadding(10);
    m_frame->setFrameFormat(frameFormat);
    QTextCharFormat format = firstCursorPosition().blockCharFormat();
    format.setProperty(Cantor::DefaultHighlighter::BlockTypeProperty, Cantor::DefaultHighlighter::NoHighlightBlock);
    firstCursorPosition().setBlockCharFormat(format);
}

TextEntry::~TextEntry()
{

}

int TextEntry::type()
{
    return Type;
}

QTextCursor TextEntry::firstValidCursorPosition()
{
    return firstCursorPosition();
}

QTextCursor TextEntry::lastValidCursorPosition()
{
    return lastCursorPosition();
}

QTextCursor TextEntry::closestValidCursor(const QTextCursor& cursor)
{
    return QTextCursor(cursor);
}

bool TextEntry::isValidCursor(const QTextCursor& cursor)
{
    int pos = cursor.position();
    return (firstValidPosition() <= pos && pos <= lastValidPosition());
}

bool TextEntry::isEmpty()
{
    QTextCursor cursor = firstValidCursorPosition();
    cursor.setPosition(lastValidPosition(), QTextCursor::KeepAnchor);
    return cursor.selection().isEmpty();
}

bool TextEntry::worksheetMouseDoubleClickEvent(QMouseEvent* event, const QTextCursor& cursor)
{
    QTextCursor c=cursor;

    for(int pos=cursor.selectionStart()+1;pos<=cursor.selectionEnd();pos++)
    {
        c.setPosition(pos);
        if (c.charFormat().objectType() == FormulaTextObject::FormulaTextFormat)
            showLatexCode(c);
    }
    return true;
}

bool TextEntry::acceptRichText()
{
    return true;
}

bool TextEntry::acceptsDrop(const QTextCursor& cursor)
{
    Q_UNUSED(cursor);

    return true;
}

bool TextEntry::worksheetContextMenuEvent(QContextMenuEvent* event, const QTextCursor& cursor)
{
    Q_UNUSED(cursor);
    KMenu* defaultMenu = new KMenu(m_worksheet);

    defaultMenu->addAction(KStandardAction::cut(m_worksheet));
    defaultMenu->addAction(KStandardAction::copy(m_worksheet));
    defaultMenu->addAction(KStandardAction::paste(m_worksheet));
    defaultMenu->addSeparator();

    if(!m_worksheet->isRunning())
	defaultMenu->addAction(KIcon("system-run"),i18n("Evaluate Worksheet"),m_worksheet,SLOT(evaluate()),0);
    else
	defaultMenu->addAction(KIcon("process-stop"),i18n("Interrupt"),m_worksheet,SLOT(interrupt()),0);
    defaultMenu->addSeparator();

    defaultMenu->addAction(KIcon("edit-delete"),i18n("Remove Entry"), m_worksheet, SLOT(removeCurrentEntry()));

    createSubMenuInsert(defaultMenu);

    defaultMenu->popup(event->globalPos());

    return true;
}


void TextEntry::setContent(const QString& content)
{
    firstValidCursorPosition().insertText(content);
}

void TextEntry::setContent(const QDomElement& content, const KZip& file)
{
    Q_UNUSED(file);
    if(content.firstChildElement("body").isNull())
	return;

    QDomDocument doc = QDomDocument();
    QDomNode n = doc.importNode(content.firstChildElement("body"), true);
    doc.appendChild(n);
    QString html = doc.toString();
    kDebug() << html;
    firstValidCursorPosition().insertHtml(html);
}

QDomElement TextEntry::toXml(QDomDocument& doc, KZip* archive)
{
    Q_UNUSED(archive);

    QTextCursor cursor = firstValidCursorPosition();
    cursor.setPosition(lastValidPosition(), QTextCursor::KeepAnchor);
    const QString& html = cursor.selection().toHtml();
    kDebug() << html;
    QDomElement el = doc.createElement("Text");
    QDomDocument myDoc = QDomDocument();
    myDoc.setContent(html);
    el.appendChild(myDoc.documentElement().firstChildElement("body"));
    return el;
}

QString TextEntry::toPlain(QString& commandSep, QString& commentStartingSeq, QString& commentEndingSeq)
{
    Q_UNUSED(commandSep);

    if (commentStartingSeq.isEmpty())
        return QString();
    QTextCursor cursor = firstValidCursorPosition();
    cursor.setPosition(lastValidPosition(), QTextCursor::KeepAnchor);
    QString text = cursor.selection().toPlainText();
    if (!commentEndingSeq.isEmpty())
        return commentStartingSeq + text + commentEndingSeq + "\n";
    return commentStartingSeq + text.replace("\n", "\n" + commentStartingSeq) + "\n";
}

void TextEntry::interruptEvaluation()
{

}

bool TextEntry::evaluate(bool current)
{
    Q_UNUSED(current);

    QTextDocument *doc = m_frame->document();
    //blahtex::Interface *translator = m_worksheet->translator();

    QTextCursor cursor = findLatexCode(doc);
    while (!cursor.isNull())
    {
        QString latexCode = cursor.selectedText();
        kDebug()<<"found latex: "<<latexCode;

        latexCode.remove(0, 2);
        latexCode.remove(latexCode.length() - 2, 2);


        Cantor::LatexRenderer* renderer=new Cantor::LatexRenderer(this);
        renderer->setLatexCode(latexCode);
        renderer->setEquationOnly(true);
        renderer->setEquationType(Cantor::LatexRenderer::InlineEquation);
        renderer->setMethod(Cantor::LatexRenderer::LatexMethod);

        renderer->renderBlocking();

#if 0
        QTextCharFormat mmlCharFormat;
        mmlCharFormat.setObjectType(MmlTextObject::MmlTextFormat);
        mmlCharFormat.setProperty(MmlTextObject::LatexCode, latexCode);

        if (!translator->ProcessInput(latexCode.toStdWString()))
            break;
        QString mmlCode = QString::fromStdWString(translator->GetMathml());
        kDebug() << mmlCode;
        if (mmlCode.isEmpty())
            break;

        QtMmlDocument renderer;
        renderer.setBaseFontPointSize(cursor.charFormat().fontPointSize());
        renderer.setFontName(QtMmlWidget::NormalFont, "STIXGeneral");
        renderer.setContent(mmlCode);

        QImage mmlBufferImage(renderer.size(), QImage::Format_ARGB32);
        mmlBufferImage.fill(QColor(0, 0, 0, 0).value());
        QPainter painter(&mmlBufferImage);
        renderer.paint(&painter, mmlBufferImage.rect().topLeft ());

        mmlCharFormat.setProperty(MmlTextObject::MmlData, mmlBufferImage);

        cursor.removeSelectedText();
        cursor.insertText(QString(QChar::ObjectReplacementCharacter), mmlCharFormat);

#endif

        bool success=m_worksheet->resultProxy()->renderEpsToResource(renderer->imagePath());
        kDebug()<<"rendering successfull? "<<success;

        QString path=renderer->imagePath();
        KUrl internal=KUrl(path);
        internal.setProtocol("internal");
        kDebug()<<"int: "<<internal;

        QTextCharFormat formulaFormat;
        formulaFormat.setObjectType(FormulaTextObject::FormulaTextFormat);
        formulaFormat.setProperty( FormulaTextObject::Data,renderer->imagePath());
        formulaFormat.setProperty( FormulaTextObject::ResourceUrl, internal);
        formulaFormat.setProperty( FormulaTextObject::LatexCode, latexCode);
        formulaFormat.setProperty( FormulaTextObject::FormulaType, renderer->method());

        cursor.insertText(QString(QChar::ObjectReplacementCharacter), formulaFormat);
        delete renderer;

        cursor = findLatexCode(doc);

    }

    return true;
}

void TextEntry::update()
{
    QTextCursor cursor = m_worksheet->document()->find(QString(QChar::ObjectReplacementCharacter), m_frame->firstCursorPosition());
    while(!cursor.isNull()&&cursor.position()<=m_frame->lastPosition())
    {
        QTextCharFormat format=cursor.charFormat();
        if (format.objectType() == FormulaTextObject::FormulaTextFormat)
        {
            kDebug()<<"found a formula... rendering the eps...";
            QUrl url=qVariantValue<QUrl>(format.property(FormulaTextObject::Data));
            bool success=m_worksheet->resultProxy()->renderEpsToResource(url);
            kDebug()<<"rendering successfull? "<<success;

            //HACK: reinsert this image, to make sure the layout is updated to the new size
            cursor.deletePreviousChar();
            cursor.insertText(QString(QChar::ObjectReplacementCharacter), format);
        }

        cursor = m_worksheet->document()->find(QString(QChar::ObjectReplacementCharacter), cursor);
    }

}

QTextCursor TextEntry::findLatexCode(QTextDocument *doc) const
{
    QTextCursor startCursor = doc->find("$$");
    if (startCursor.isNull())
        return startCursor;
    const QTextCursor endCursor = doc->find("$$", startCursor);
    if (endCursor.isNull())
        return endCursor;
    startCursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, 2);
    startCursor.setPosition(endCursor.position(), QTextCursor::KeepAnchor);
    return startCursor;
}

void TextEntry::showLatexCode(QTextCursor cursor)
{
    QString latexCode = qVariantValue<QString>(cursor.charFormat().property(FormulaTextObject::LatexCode));
    cursor.deletePreviousChar();
    cursor.insertText("$$"+latexCode+"$$");
}
