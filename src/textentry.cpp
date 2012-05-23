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

#include "textentry.h"
#include "worksheettextitem.h"
#include "animationhandler.h"
#include "epsrenderer.h"
#include "latexrenderer.h"

#include <QGraphicsLinearLayout>

#include <kdebug.h>
#include <KUrl>
#include <KLocale>

TextEntry::TextEntry(Worksheet* worksheet) : WorksheetEntry(worksheet), m_textItem(new WorksheetTextItem(this, Qt::TextEditorInteraction))
{
    connect(m_textItem, SIGNAL(moveToPrevious(int, qreal)),
	    this, SLOT(moveToPreviousEntry(int, qreal)));
    connect(m_textItem, SIGNAL(moveToNext(int, qreal)),
	    this, SLOT(moveToNextEntry(int, qreal)));
    connect(m_textItem, SIGNAL(execute()), this, SLOT(evaluate()));
    connect(m_textItem, SIGNAL(doubleClick()), this, SLOT(resolveImagesAtCursor()));
}

TextEntry::~TextEntry()
{
}

void TextEntry::populateMenu(KMenu *menu)
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
		cursor.charFormat().hasProperty(EpsRenderer::CantorFormula)) {
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

bool TextEntry::isEmpty()
{
    return m_textItem->document()->isEmpty();
}

int TextEntry::type() const
{
    return Type;
}

bool TextEntry::acceptRichText()
{
    return true;
}

/*
void TextEntry::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    QTextCursor c;

    for (int pos = m_textItem->textCursor().selectionStart()+1;
	 pos <= m_textItem->textCursor().selectionEnd(); ++pos)
    {
	c.setPosition(pos);
        if (c.charFormat().hasProperty(EpsRenderer::CantorFormula))
            showLatexCode(c);
    }
}
*/

bool TextEntry::focusEntry(int pos, qreal xCoord)
{
    m_textItem->setFocusAt(pos, xCoord);
    return true;
}


void TextEntry::setContent(const QString& content)
{
    m_textItem->setPlainText(content);
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
    m_textItem->setHtml(html);
}

QDomElement TextEntry::toXml(QDomDocument& doc, KZip* archive)
{
    Q_UNUSED(archive);

    bool needsEval=false;
    //make sure that the latex code is shown instead of the rendered formulas
    QTextCursor cursor = m_textItem->document()->find(QString(QChar::ObjectReplacementCharacter));
    while(!cursor.isNull())
    {
        QTextCharFormat format = cursor.charFormat();
        if (format.hasProperty(EpsRenderer::CantorFormula))
        {
            showLatexCode(cursor);
            needsEval=true;
        }

        cursor = m_textItem->document()->find(QString(QChar::ObjectReplacementCharacter), cursor);
    }

    const QString& html = m_textItem->toHtml();
    kDebug() << html;
    QDomElement el = doc.createElement("Text");
    QDomDocument myDoc = QDomDocument();
    myDoc.setContent(html);
    el.appendChild(myDoc.documentElement().firstChildElement("body"));

    if (needsEval)
	evaluate(false);
    return el;
}

QString TextEntry::toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq)
{
    Q_UNUSED(commandSep);

    if (commentStartingSeq.isEmpty())
        return QString();
    QString text = m_textItem->toPlainText();
    if (!commentEndingSeq.isEmpty())
        return commentStartingSeq + text + commentEndingSeq + "\n";
    return commentStartingSeq + text.replace("\n", "\n" + commentStartingSeq) + "\n";
    
}

void TextEntry::interruptEvaluation()
{
}

bool TextEntry::evaluate(int evalOp)
{
    QTextCursor cursor = findLatexCode();
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

        bool success;
	QTextImageFormat formulaFormat;
	if (renderer->renderingSuccessful()) {
	    EpsRenderer* epsRend = worksheet()->epsRenderer();
	    formulaFormat = epsRend->renderEps(m_textItem->document(), renderer);
	    success = !formulaFormat.name().isEmpty();
	} else {
	    success = false;
	}

        kDebug()<<"rendering successfull? "<<success;
	if (!success) {
	    cursor = findLatexCode(cursor);
	    continue;
	}

        formulaFormat.setProperty(EpsRenderer::Delimiter, "$$");

        cursor.insertText(QString(QChar::ObjectReplacementCharacter), formulaFormat);
        delete renderer;

        cursor = findLatexCode(cursor);
    }

    recalculateSize();
    evaluateNext(evalOp);

    return true;
}

void TextEntry::updateEntry()
{
    kDebug() << "update Entry";
    QTextCursor cursor = m_textItem->document()->find(QString(QChar::ObjectReplacementCharacter));
    while(!cursor.isNull())
    {
	kDebug() << "orc at" << cursor.position();
        QTextCharFormat format = cursor.charFormat();
        if (format.hasProperty(EpsRenderer::CantorFormula))
        {
            kDebug() << "found a formula... rendering the eps...";
            QUrl url=qVariantValue<QUrl>(format.property(EpsRenderer::ImagePath));
            QSize s = worksheet()->epsRenderer()->renderEpsToResource(m_textItem->document(), url);
            kDebug() << "rendering successfull? " << s.isValid();

            //cursor.deletePreviousChar();
            //cursor.insertText(QString(QChar::ObjectReplacementCharacter), format);
        }

        cursor = m_textItem->document()->find(QString(QChar::ObjectReplacementCharacter), cursor);
    }
    recalculateSize();
}

void TextEntry::resolveImagesAtCursor()
{
    QTextCursor cursor = m_textItem->textCursor();
    if (!cursor.hasSelection())
	cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
    cursor.insertText(m_textItem->resolveImages(cursor));
    recalculateSize();
}

QTextCursor TextEntry::findLatexCode(QTextCursor cursor) const
{
    QTextDocument *doc = m_textItem->document();
    QTextCursor startCursor;
    if (cursor.isNull())
	startCursor = doc->find("$$");
    else
	startCursor = doc->find("$$", cursor);
    if (startCursor.isNull())
        return startCursor;
    const QTextCursor endCursor = doc->find("$$", startCursor);
    if (endCursor.isNull())
        return endCursor;
    startCursor.setPosition(startCursor.selectionStart());
    startCursor.setPosition(endCursor.position(), QTextCursor::KeepAnchor);
    return startCursor;
}

QString TextEntry::showLatexCode(QTextCursor cursor)
{
    QString latexCode = qVariantValue<QString>(cursor.charFormat().property(EpsRenderer::Code));
    cursor.deletePreviousChar();
    latexCode = "$$"+latexCode+"$$";
    cursor.insertText(latexCode);
    return latexCode;
}

void TextEntry::layOutForWidth(double w, bool force)
{
    if (entrySize().width() == w && !force)
	return;

    m_textItem->setPos(0,0);
    m_textItem->setTextWidth(w);
    setEntrySize(QSizeF(w, m_textItem->height()));
}

bool TextEntry::wantToEvaluate()
{
    return !findLatexCode().isNull();
}
