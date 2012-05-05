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

#include "latexentry.h"

#include "worksheetentry.h"
#include "worksheet.h"
#include "resultproxy.h"
#include "lib/defaulthighlighter.h"
#include "lib/latexrenderer.h"

#include "formulatextobject.h"

LatexEntry::LateyEntry() : WorksheetENtry(). m_textitem(new WorksheetTextItem(this))
{
    m_textItem.setTextIteractionFlags(Qt::TextEditorInteraction);
    // ToDo: pass information about the desired cursor position.
    connect(m_textItem, SIGNAL(leftmostValidPositionReached()), 
	    worksheet(), SLOT(moveToPreviousEntry()));
    connect(m_textItem, SIGNAL(rightmostValidPositionReached()), 
	    worksheet(), SLOT(moveToNextEntry()));
    connect(m_textItem, SIGNAL(topmostValidLineReached()), 
	    worksheet(), SLOT(moveToPreviousEntry()));
    connect(m_textItem, SIGNAL(bottommostValidLineReached()), 
	    worksheet(), SLOT(moveToNextEntry()));
}

LatexEntry::~LatexEntry()
{
}

int LatexEntry::type()
{
    return type;
}

bool LatexEntry::isEmpty()
{
    return m_textItem.document().isEmpty();
}

bool LatexEntry::acceptRichText()
{
    return false;
}

void LatexEntry::setContent(const QString& content)
{
    m_textItem.setPlainText(content);
}

void LatexEntry::setContent(const QDomElement& content, const KZip& file)
{
    QString latexCode = content.text();
    kDebug() << latexCode;

    m_textItem->document()->clear();
    QTextCursor cursor = m_textItem->textCursor();
    cursor.movePosition(QTextCursor::Start);

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
        } else
        {
            cursor.insertText(latexCode);
        }
    } else
    {
        cursor.insertText(latexCode);
    }
}



QDomElement LatexEntry::toXml(QDomDocument& doc, KZip* archive)
{
    Q_UNUSED(archive);

    QString image;

    QString latex = latexCode();

    if (isOneImageOnly()) 
    {
	QTextCursor cursor = m_textItem->textCursor();

        if(cursor.charFormat().intProperty(FormulaTextObject::FormulaType) == FormulaTextObject::LatexFormula)
            image = qVariantValue<QString>(cursor.charFormat().property(FormulaTextObject::Data));
    }

    QDomElement el = doc.createElement("Latex");

    if(!image.isNull())
    {
        KUrl url(image);
        el.setAttribute("filename", url.fileName());
        archive->addLocalFile(image, url.fileName());
    }

    kDebug() << latex;
    QDomText text = doc.createTextNode(latex);

    el.appendChild(text);

    return el;
}

QString LatexEntry::toPlain(QString& commandSep, QString& commentStartingSeq, QString& commentEndingSeq)
{
    Q_UNUSED(commandSep);

    if (commentStartingSeq.isEmpty())
        return QString();

    QString text = latexCode();
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

    if (isOneImageOnly())
	return true; // the image is rendered already

    QTextDocument *doc = m_frame->document();
    QString latex = latexCode();

    Cantor::LatexRenderer* renderer = new Cantor::LatexRenderer(this);
    renderer->setLatexCode(latex);
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
    formulaFormat.setProperty( FormulaTextObject::LatexCode, latex);
    formulaFormat.setProperty( FormulaTextObject::FormulaType, renderer->method());

    QTextCursor cursor = m_textItem->textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.insertText(QString(QChar::ObjectReplacementCharacter), formulaFormat);
    delete renderer;

    return true;
}

void LatexEntry::updateEntry()
{
    QTextCursor cursor = m_textItem->document()->find(QString(QChar::ObjectReplacementCharacter));
    while (!cursor.isNull())
    {
        kDebug()<<"found a formula... rendering the eps...";
        QTextCharFormat format=cursor.charFormat();
        QUrl url=qVariantValue<QUrl>(format.property(FormulaTextObject::Data));
        bool success=m_worksheet->resultProxy()->renderEpsToResource(url);
        kDebug()<<"rendering successfull? "<<success;

        //HACK: reinsert this image, to make sure the layout is updated to the new size
        cursor.removeSelectedText();
        cursor.insertText(QString(QChar::ObjectReplacementCharacter), format);
	cursor.movePosition(QTextCursor::NextCharacter);

	cursor = m_textItem->document()->find(QString(QChar::ObjectReplacementCharacter), cursor);
    }
}

QString LatexEntry::latexCode()
{
    QString latex = "";
    
    QTextCursor cursor1 = m_textItem->textCursor();
    cursor1.movePosition(QTextCursor::Start);
    QTextCursor cursor2 = m_textItem->document()->find(QString(QChar::ObjectReplacementCharacter));

    // find all rendered images, and concatenate the latex code
    for(; !cursor2.isNull(); cursor2 = m_textItem->document()->find(QString(QChar::ObjectReplacementCharacter), cursor1)) 
    {
	cursor1.setPosition(cursor2.selectionStart(), QTextCursor::KeepAnchor);
	latex += cursor1.selectedText();
	latex += qVariantValue<QString>(cursor2.charFormat().property(FormulaTextObject::LatexCode));

	cursor1.setPosition(cursor2.selectionEnd());
    }

    cursor1.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    latex += cursor1.selectedText();
  
    return latex;
}

bool LatexEntry::isOneImageOnly()
{
    QTextCursor cursor = m_textItem->textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

    return (cursor.selectionEnd() == 1 && cursor.selectedText() == QString(QChar::ObjectReplacementCharacter));
}
