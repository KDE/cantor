
#include "worksheetentry.h"

WorksheetEntry::WorksheetEntry() : QGraphicsWidget()
{
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

Worksheet* WorksheetEntry::worksheet()
{
    return qobject_cast<Worksheet*>(scene());
}





#include "worksheetentry.moc"
