/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#include "pagebreakentry.h"

#include <QTextCursor>
#include <QTextCharFormat>
#include <QPalette>
#include <QJsonValue>
#include <QJsonObject>
#include <KColorScheme>
#include <KLocalizedString>

#include "lib/jupyterutils.h"

PageBreakEntry::PageBreakEntry(Worksheet* worksheet)
  : WorksheetEntry(worksheet)
{
    m_msgItem = new WorksheetTextItem(this);

    QTextCursor cursor = m_msgItem->textCursor();
    KColorScheme color = KColorScheme(QPalette::Normal, KColorScheme::View);
    QTextCharFormat cformat(cursor.charFormat());
    cformat.setForeground(color.foreground(KColorScheme::InactiveText));

    cursor.insertText(i18n("--- Page Break ---"), cformat);
    m_msgItem->setAlignment(Qt::AlignCenter);

    setFlag(QGraphicsItem::ItemIsFocusable);
}

bool PageBreakEntry::isEmpty()
{
    return false;
}

int PageBreakEntry::type() const
{
    return Type;
}

void PageBreakEntry::populateMenu(QMenu* menu, QPointF pos)
{
    WorksheetEntry::populateMenu(menu, pos);
}

bool PageBreakEntry::acceptRichText()
{
    return false;
}

void PageBreakEntry::setContent(const QString& content)
{
    Q_UNUSED(content);
    return;
}

void PageBreakEntry::setContent(const QDomElement& content, const KZip& file)
{
    Q_UNUSED(content);
    Q_UNUSED(file);
    return;
}

void PageBreakEntry::setContentFromJupyter(const QJsonObject& cell)
{
    Q_UNUSED(cell);
    return;
}

QJsonValue PageBreakEntry::toJupyterJson()
{
    QJsonObject root;

    root.insert(QLatin1String("cell_type"), QLatin1String("raw"));
    QJsonObject metadata;

    // "raw_mimetype" vs "format"?
    // See https://github.com/jupyter/notebook/issues/4730
    // For safety set both keys
    metadata.insert(QLatin1String("format"), QLatin1String("text/latex"));
    metadata.insert(QLatin1String("raw_mimetype"), QLatin1String("text/latex"));

    QJsonObject cantor;
    cantor.insert(QLatin1String("from_page_break"), true);
    metadata.insert(Cantor::JupyterUtils::cantorMetadataKey, cantor);

    root.insert(Cantor::JupyterUtils::metadataKey, metadata);
    Cantor::JupyterUtils::setSource(root, QLatin1String("\\pagebreak"));

    return root;
}

QDomElement PageBreakEntry::toXml(QDomDocument& doc, KZip* archive)
{
    Q_UNUSED(archive);

    QDomElement pgbrk = doc.createElement(QLatin1String("PageBreak"));
    return pgbrk;
}

QString PageBreakEntry::toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq)
{
    Q_UNUSED(commandSep);

    return commentStartingSeq + QLatin1String("page break") + commentEndingSeq;
}

void PageBreakEntry::interruptEvaluation()
{
    return;
}

void PageBreakEntry::layOutForWidth(qreal entry_zone_x, qreal w, bool force)
{
    if (size().width() == w && m_msgItem->pos().x() == entry_zone_x && !force)
        return;

    const qreal margin = worksheet()->isPrinting() ? 0 : RightMargin;

    if (m_msgItem->isVisible()) {
        m_msgItem->setGeometry(entry_zone_x, 0, w - margin - entry_zone_x, true);

        setSize(QSizeF(m_msgItem->width() + margin + entry_zone_x, m_msgItem->height() + VerticalMargin));
    } else {
        setSize(QSizeF(w, 0));
    }
}

bool PageBreakEntry::evaluate(EvaluationOption evalOp)
{
    evaluateNext(evalOp);
    return true;
}

void PageBreakEntry::updateEntry()
{
    if (worksheet()->isPrinting()) {
        m_msgItem->setVisible(false);
        recalculateSize();
    } else if (!m_msgItem->isVisible()) {
        m_msgItem->setVisible(true);
        recalculateSize();
    }
    return;
}

/*
void PageBreakEntry::paint(QPainter* painter, const QStyleOptionGraphicsItem*,
                           QWidget*)
{
    if (worksheet()->isPrinting()) {
        QPaintDevice* device = painter->paintEngine()->paintDevice();
        QPrinter* printer = qobject_cast<QPrinter*>(device);
        if (printer)
            printer->newPage();
    }
}
*/

bool PageBreakEntry::wantToEvaluate()
{
    return false;
}

bool PageBreakEntry::wantFocus()
{
    return false;
}

bool PageBreakEntry::isConvertableToPageBreakEntry(const QJsonObject& cell)
{
    if (!Cantor::JupyterUtils::isRawCell(cell))
        return false;

    QJsonObject metadata = Cantor::JupyterUtils::getCantorMetadata(cell);
    QJsonValue value = metadata.value(QLatin1String("from_page_break"));

    return value.isBool() && value.toBool() == true;
}

