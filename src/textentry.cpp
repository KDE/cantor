
#include "textentry.h"
#include "animationhandler.h"
#include "formulatextobject.h"

TextEntry::TextEntry() : WorksheetEntry(), m_textItem(new WorksheetItem(this))
{
    m_textItem.document()->documentLayout()->registerHandler(QTextFormat::ImageObject, new AnimationHandler(document()));
    m_textItem.document()->documentLayout()->registerHandler(FormulaTextObject::FormulaTextFormat, new FormulaTextObject());

    setFlag(QGraphicsItem::ItemIsFocusable);
    setFocusProxy(&m_textItem);

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
    connect(m_textItem, SIGNAL(receivedFocus(QTextDocument*)),
	    worksheet(), SLOT(highlightDocument(QTextDocument*)));
}

TextEntry::~TextEntry()
{
}

bool TextEntry::isEmpty()
{
    return m_textItem.decument().isEmpty();
}

int TextEntry::type() const
{
    return Type;
}

bool TextEntry::acceptRichText()
{
    return true;
}

bool TextEntry::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QTextCursor c;

    for (int pos = m_textItem.textCursor().selectionStart()+1;
	 pos <= m_textItem.textCursor().selectionEnd(); ++pos)
    {
	c.setPosition(pos);
        if (c.charFormat().objectType() == FormulaTextObject::FormulaTextFormat)
            showLatexCode(c);
    }
    return true;
}

void TextEntry::setContent(const QString& content)
{
    m_textItem.setPlainText(content);
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
    m_textItem.setHtml(html);
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
        if (format.objectType() == FormulaTextObject::FormulaTextFormat)
        {
            showLatexCode(cursor);
            needsEval=true;
        }

        cursor = m_worksheet->document()->find(QString(QChar::ObjectReplacementCharacter), cursor);
    }

    const QString& html = m_textItem.toHtml();
    kDebug() << html;
    QDomElement el = doc.createElement("Text");
    QDomDocument myDoc = QDomDocument();
    myDoc.setContent(html);
    el.appendChild(myDoc.documentElement().firstChildElement("body"));

    if (needsEval)
	evaluate(false);
    return el;
}

QString TextEntry::toPlain(QString& commandSep, QString& commentStartingSeq, 
			   QString& commentEndingSeq)
{
    Q_UNUSED(commandSep);

    if (commentStartingSeq.isEmpty())
        return QString();
    QString text = m_textItem.toPlainText();
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

    QTextDocument *doc = m_textItem->document();
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

void TextEntry::updateEntry()
{
    QTextCursor cursor = m_textItem->document()->find(QString(QChar::ObjectReplacementCharacter));
    while(!cursor.isNull())
    {
        QTextCharFormat format = cursor.charFormat();
        if (format.objectType() == FormulaTextObject::FormulaTextFormat)
        {
            kDebug() << "found a formula... rendering the eps...";
            QUrl url=qVariantValue<QUrl>(format.property(FormulaTextObject::Data));
            bool success=m_worksheet->resultProxy()->renderEpsToResource(url);
            kDebug() << "rendering successfull? " << success;

            //HACK: reinsert this image, to make sure the layout is updated to the new size
            cursor.deletePreviousChar();
            cursor.insertText(QString(QChar::ObjectReplacementCharacter), format);
        }

        cursor = m_textItem->document()->find(QString(QChar::ObjectReplacementCharacter), cursor);
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
