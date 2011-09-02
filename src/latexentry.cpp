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

#include "latexentry.h"

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
#include <kstandarddirs.h>


LatexEntry::LatexEntry(QTextCursor position, Worksheet* parent ) : WorksheetEntry( position, parent )
{
    QTextFrameFormat frameFormat = m_frame->frameFormat();
    frameFormat.setPadding(10);
    m_frame->setFrameFormat(frameFormat);
    QTextCharFormat format = firstCursorPosition().blockCharFormat();
    format.setProperty(Cantor::DefaultHighlighter::BlockTypeProperty, Cantor::DefaultHighlighter::NoHighlightBlock);
    firstCursorPosition().setBlockCharFormat(format);
}

LatexEntry::~LatexEntry()
{

}

int LatexEntry::type()
{
    return Type;
}

QTextCursor LatexEntry::firstValidCursorPosition()
{
    return firstCursorPosition();
}

QTextCursor LatexEntry::lastValidCursorPosition()
{
    return lastCursorPosition();
}

QTextCursor LatexEntry::closestValidCursor(const QTextCursor& cursor)
{
    return QTextCursor(cursor);
}

bool LatexEntry::isValidCursor(const QTextCursor& cursor)
{
    int pos = cursor.position();
    return (firstValidPosition() <= pos && pos <= lastValidPosition());
}

bool LatexEntry::isEmpty()
{
    QTextCursor cursor = firstValidCursorPosition();
    cursor.setPosition(lastValidPosition(), QTextCursor::KeepAnchor);
    return cursor.selection().isEmpty();
}

bool LatexEntry::worksheetMouseDoubleClickEvent(QMouseEvent* event, const QTextCursor& /*cursor*/)
{
    if(!m_isShowingCode)
    {
        m_isShowingCode=true;
        QTextCursor cursor=firstValidCursorPosition();
        QString code=qVariantValue<QString>(cursor.charFormat().property(FormulaTextObject::LatexCode));
        kDebug()<<"code: "<<code;
        cursor.setPosition(lastValidPosition(), QTextCursor::KeepAnchor);

        cursor.removeSelectedText();
        cursor.insertText(code);
    }

    return true;
}

bool LatexEntry::acceptRichText()
{
    return false;
}

bool LatexEntry::acceptsDrop(const QTextCursor& cursor)
{
    Q_UNUSED(cursor);

    return true;
}

bool LatexEntry::worksheetContextMenuEvent(QContextMenuEvent* event, const QTextCursor& cursor)
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


void LatexEntry::setContent(const QString& content)
{
    firstValidCursorPosition().insertText(content);
}

void LatexEntry::setContent(const QDomElement& content, const KZip& file)
{
    QString latexCode= content.text();
    kDebug() << latexCode;

    QTextCursor cursor=firstValidCursorPosition();
    cursor.setPosition(lastValidPosition(), QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    cursor=firstValidCursorPosition();

    if(content.hasAttribute("filename"))
    {
        const KArchiveEntry* imageEntry=file.directory()->entry(content.attribute("filename"));
        if (imageEntry&&imageEntry->isFile())
        {
            const KArchiveFile* imageFile=static_cast<const KArchiveFile*>(imageEntry);
            QString dir=KGlobal::dirs()->saveLocation("tmp", "cantor/");
            imageFile->copyTo(dir);
            QString imagePath=QString(dir+QLatin1Char('/')+imageFile->name());

            KUrl internal=KUrl(imagePath);
            internal.setProtocol("internal");

            bool success=m_worksheet->resultProxy()->renderEpsToResource(imagePath);
            kDebug()<<"rendering successfull? "<<success;

            QTextCharFormat formulaFormat;
            formulaFormat.setObjectType(FormulaTextObject::FormulaTextFormat);
            formulaFormat.setProperty( FormulaTextObject::Data,imagePath);
            formulaFormat.setProperty( FormulaTextObject::ResourceUrl, internal);
            formulaFormat.setProperty( FormulaTextObject::LatexCode, latexCode);
            formulaFormat.setProperty( FormulaTextObject::FormulaType, Cantor::LatexRenderer::LatexMethod); //So far only latex is supported

            cursor.insertText(QString(QChar::ObjectReplacementCharacter), formulaFormat);
            m_isShowingCode=false;
        }else
        {
            cursor.insertText(latexCode);
            m_isShowingCode=true;
        }
    }else
    {
        cursor.insertText(latexCode);
        m_isShowingCode=true;
    }
}

QDomElement LatexEntry::toXml(QDomDocument& doc, KZip* archive)
{
    Q_UNUSED(archive);

    QString html;
    QString image;
    if(m_isShowingCode)
    {
        QTextCursor cursor = firstValidCursorPosition();
        cursor.setPosition(lastValidPosition(), QTextCursor::KeepAnchor);
        html = cursor.selectedText();
    }else
    {
        QTextCursor cursor=firstValidCursorPosition();
        html=qVariantValue<QString>(cursor.charFormat().property(FormulaTextObject::LatexCode));
        if(cursor.charFormat().intProperty(FormulaTextObject::FormulaType)==FormulaTextObject::LatexFormula)
            image=qVariantValue<QString>(cursor.charFormat().property(FormulaTextObject::Data));
    }

    QDomElement el = doc.createElement("Latex");

    if(!image.isNull())
    {
        KUrl url(image);
        el.setAttribute("filename", url.fileName());
        archive->addLocalFile(image, url.fileName());
    }

    kDebug() << html;
    QDomText text=doc.createTextNode(html);

    el.appendChild(text);

    return el;
}

QString LatexEntry::toPlain(QString& commandSep, QString& commentStartingSeq, QString& commentEndingSeq)
{
    Q_UNUSED(commandSep);

    if (commentStartingSeq.isEmpty())
        return QString();
    QString text;
    if(m_isShowingCode)
    {
        QTextCursor cursor = firstValidCursorPosition();
        cursor.setPosition(lastValidPosition(), QTextCursor::KeepAnchor);
        text = cursor.selection().toPlainText();
    }else
    {
        QTextCursor cursor=firstValidCursorPosition();
        text=qVariantValue<QString>(cursor.charFormat().property(FormulaTextObject::LatexCode));
    }
    if (!commentEndingSeq.isEmpty())
        return commentStartingSeq + text + commentEndingSeq + "\n";
    return commentStartingSeq + text.replace("\n", "\n" + commentStartingSeq) + "\n";
}

void LatexEntry::interruptEvaluation()
{

}

bool LatexEntry::evaluate(bool current)
{
    Q_UNUSED(current);

    QTextDocument *doc = m_frame->document();
    QTextCursor cursor=firstValidCursorPosition();
    cursor.setPosition(lastValidPosition(), QTextCursor::KeepAnchor);
    QString latexCode=cursor.selection().toPlainText();
    cursor.removeSelectedText();

    Cantor::LatexRenderer* renderer=new Cantor::LatexRenderer(this);
    renderer->setLatexCode(latexCode);
    renderer->setEquationOnly(false);
    renderer->setMethod(Cantor::LatexRenderer::LatexMethod);

    renderer->renderBlocking();

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

    m_isShowingCode=false;

    return true;
}

void LatexEntry::update()
{
    if(!m_isShowingCode)
    {
        kDebug()<<"found a formula... rendering the eps...";
        QTextCursor cursor=firstValidCursorPosition();
        QTextCharFormat format=cursor.charFormat();
        QUrl url=qVariantValue<QUrl>(format.property(FormulaTextObject::Data));
        bool success=m_worksheet->resultProxy()->renderEpsToResource(url);
        kDebug()<<"rendering successfull? "<<success;

        //HACK: reinsert this image, to make sure the layout is updated to the new size
        cursor.deletePreviousChar();
        cursor.insertText(QString(QChar::ObjectReplacementCharacter), format);

        cursor = m_worksheet->document()->find(QString(QChar::ObjectReplacementCharacter), cursor);
    }
}



