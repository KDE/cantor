
#include "worksheetentry.h"

WorksheetEntry::WorksheetEntry() : QGraphicsWidget()
{
    m_next = 0;
    m_prev = 0;

    connect(this, SIGNAL(destroyed(QObject*)), m_worksheet, SLOT(removeEntry(QObject*)));
}

WorksheetEntry::~WorksheetEntry()
{
}

int WorksheetEntry::type() const
{
    return Type;
}


static Worksheet* WorksheetEntry::create(int t)
{
    switch(t)
    {
    case TextEntry::Type:
	return new TextEntry;
    case CommandEntry::Type:
	return new CommandEntry;
    case ImageEntry::Type:
	return new ImageEntry;
    case PageBreakEntry::Type:
	return new PageBreakEntry;
    case LatexEntry::Type:
	return new LatexEntry;
    default:
	return 0;
    }
}

WorksheetEntry* WorksheetEntry::next() const
{
    return m_next;
}

WorksheetEntry* WorksheetEntry::previous() const
{
    return m_prev;
}

void WorksheetEntry::setNext(WorksheetEntry* n)
{
    m_next = n;
}

void WorksheetEntry::setPrevious(WorksheetEntry* p)
{
    m_prev = p;
}

Worksheet* WorksheetEntry::worksheet()
{
    return qobject_cast<Worksheet*>(scene());
}





#include "worksheetentry.moc"
