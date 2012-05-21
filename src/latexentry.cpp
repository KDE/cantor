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
    Copyright (C) 2012 Martin Kuettler <martin.kuettler@gmail.com>
 */

#include "latexentry.h"

#include "worksheetentry.h"
#include "worksheet.h"
#include "resultproxy.h"
#include "formulatextobject.h"
#include "lib/defaulthighlighter.h"
#include "lib/latexrenderer.h"

#include <kdebug.h>
#include <kglobal.h>
#include <KStandardDirs>
#include <KLocale>

LatexEntry::LatexEntry(Worksheet* worksheet) : WorksheetEntry(worksheet), m_textItem(new WorksheetTextItem(this))
{
    m_textItem->document()->documentLayout()->registerHandler(FormulaTextObject::FormulaTextFormat, new FormulaTextObject());

    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(this);
    layout->addItem(m_textItem);
    setLayout(layout);
    connect(m_textItem, SIGNAL(moveToPrevious(int, qreal)),
	    this, SLOT(moveToPreviousEntry(int, qreal)));
    connect(m_textItem, SIGNAL(moveToNext(int, qreal)),
	    this, SLOT(moveToNextEntry(int, qreal)));
    connect(m_textItem, SIGNAL(execute()), this, SLOT(evaluate()));
    connect(m_textItem, SIGNAL(doubleClick()), this, SLOT(resolveImagesAtCursor()));
}

LatexEntry::~LatexEntry()
{
}

void LatexEntry::populateMenu(KMenu *menu)
{
    bool imageSelected;
    QTextCursor cursor = m_textItem->textCursor();
    const QChar repl = QChar::ObjectReplacementCharacter;
    if (cursor.hasSelection()) {
	QString selection = m_textItem->textCursor().selectedText();
	imageSelected = selection.contains(repl);
    } else {
	// we need to try both the current cursor and the one after the that
	for (int i = 2; i; --i) {
	    int p = cursor.position();
	    if (m_textItem->document()->characterAt(p-1) == repl &&
		cursor.charFormat().objectType() == FormulaTextObject::FormulaTextFormat) {
		m_textItem->setTextCursor(cursor);
		imageSelected = true;
		break;
	    }
	    cursor.movePosition(QTextCursor::NextCharacter);
	}
    }
    if (imageSelected) {
	menu->addAction(i18n("Show LaTeX code"), this, SLOT(resolveImagesAtCursor()));
	menu->addSeparator();
    }
    WorksheetEntry::populateMenu(menu);
}

int LatexEntry::type() const
{
    return Type;
}

bool LatexEntry::isEmpty()
{
    return m_textItem->document()->isEmpty();
}

bool LatexEntry::acceptRichText()
{
    return false;
}

bool LatexEntry::focusEntry(int pos, qreal xCoord)
{
    m_textItem->setFocusAt(pos, xCoord);
    return true;
}

void LatexEntry::setContent(const QString& content)
{
    m_textItem->setPlainText(content);
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

            bool success=worksheet()->resultProxy()->renderEpsToResource(m_textItem->document(), imagePath);
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

QString LatexEntry::toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq)
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

bool LatexEntry::evaluate(int evalOp)
{
    if (isOneImageOnly())
	return true; // the image is rendered already

    QString latex = latexCode();

    Cantor::LatexRenderer* renderer = new Cantor::LatexRenderer(this);
    renderer->setLatexCode(latex);
    renderer->setEquationOnly(false);
    renderer->setMethod(Cantor::LatexRenderer::LatexMethod);

    renderer->renderBlocking();

    bool success = renderer->renderingSuccessful() && worksheet()->resultProxy()->renderEpsToResource(m_textItem->document(), renderer->imagePath());
    kDebug()<<"rendering successfull? "<<success;

    if (!success) {
	delete renderer;
	evaluateNext(evalOp);
	return false;
    }

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

    m_textItem->updateGeometry();
    layout()->updateGeometry();
    evaluateNext(evalOp);

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
        bool success=worksheet()->resultProxy()->renderEpsToResource(m_textItem->document(), url);
        kDebug()<<"rendering successfull? "<<success;

        //HACK: reinsert this image, to make sure the layout is updated to the new size
        cursor.removeSelectedText();
        cursor.insertText(QString(QChar::ObjectReplacementCharacter), format);
	cursor.movePosition(QTextCursor::NextCharacter);

	cursor = m_textItem->document()->find(QString(QChar::ObjectReplacementCharacter), cursor);
    }
    m_textItem->updateGeometry();
    layout()->updateGeometry();
}

void LatexEntry::resolveImagesAtCursor()
{
    QTextCursor cursor = m_textItem->textCursor();
    if (!cursor.hasSelection())
	cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);

    cursor.insertText(m_textItem->resolveImages(cursor));
    m_textItem->updateGeometry();
    layout()->updateGeometry();
}

QString LatexEntry::latexCode()
{
    QTextCursor cursor = m_textItem->textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

    return m_textItem->resolveImages(cursor);
}

bool LatexEntry::isOneImageOnly()
{
    QTextCursor cursor = m_textItem->textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

    return (cursor.selectionEnd() == 1 && cursor.selectedText() == QString(QChar::ObjectReplacementCharacter));
}

bool LatexEntry::wantToEvaluate()
{
    return !isOneImageOnly();
}
