
#include "textentry.h"

TextEntry::TextEntry() : WorksheetEntry(), m_textItem(this)
{
    m_textItem.setTextIteractionFlags(Qt::TextEditorInteraction);
    connect(m_textItem, SIGNAL(leftmostValidPositionReached()), 
	    worksheet(), SLOT(moveToEndOfPreviousEntry()));
    connect(m_textItem, SIGNAL(rightmostValidPositionReached()), 
	    worksheet(), SLOT(moveToBeginOfNextEntry()));
    connect(m_textItem, SIGNAL(topmostValidLineReached()), 
	    worksheet(), SLOT(moveToPreviousEntry()));
    connect(m_textItem, SIGNAL(bottommostValidLineReached()), 
	    worksheet(), SLOT(moveToNextEntry()));
}

TextEntry::~TextEntry()
{
}

int TextEntry::type() const
{
    return Type;
}

bool TextEntry::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
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
    // care must be taken here when inline LaTeX code is supported
    const QString& html = m_textItem.toHtml();
    kDebug() << html;
    QDomElement el = doc.createElement("Text");
    QDomDocument myDoc = QDomDocument();
    myDoc.setContent(html);
    el.appendChild(myDoc.documentElement().firstChildElement("body"));

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
    // LaTeX related code here
    return true;
}
