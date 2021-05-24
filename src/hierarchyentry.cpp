/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2020 Nikita Sirgienko <warquark@gmail.com>
*/

#include "hierarchyentry.h"
#include "settings.h"
#include "worksheettextitem.h"
#include "worksheetview.h"
#include "lib/jupyterutils.h"

#include <QJsonObject>
#include <QRegularExpression>
#include <QDrag>
#include <QBitmap>
#include <QMimeData>
#include <QPainter>
#include <QDebug>

#include <KLocalizedString>

static QStringList hierarchyLevelNames = {i18n("Chapter"), i18n("Subchapter"), i18n("Section"), i18n("Subsection"), i18n("Paragraph"), i18n("Subparagraph")};

HierarchyEntry::HierarchyEntry(Worksheet* worksheet) : WorksheetEntry(worksheet)
    , m_hierarchyLevelItem(new WorksheetTextItem(this, Qt::NoTextInteraction))
    , m_textItem(new WorksheetTextItem(this, Qt::TextEditorInteraction))
    , m_depth(HierarchyLevel::Chapter)
    , m_hierarchyNumber(1)
    , m_hidedSubentries(nullptr)
{
    // Font and sizes should be regulated from future Settings "Styles" option
    m_textItem->enableRichText(false);

    connect(m_textItem, &WorksheetTextItem::moveToPrevious, this, &HierarchyEntry::moveToPreviousEntry);
    connect(m_textItem, &WorksheetTextItem::moveToNext, this, &HierarchyEntry::moveToNextEntry);
    // Modern syntax of signal/stots don't work on this connection (arguments don't match)
    connect(m_textItem, SIGNAL(execute()), this, SLOT(evaluate()));

    connect(this, &HierarchyEntry::hierarhyEntryNameChange, worksheet, &Worksheet::hierarhyEntryNameChange);

    connect(&m_controlElement, &WorksheetControlItem::doubleClick, this, &HierarchyEntry::handleControlElementDoubleClick);

    m_setLevelActionGroup = new QActionGroup(this);
    m_setLevelActionGroup->setExclusive(true);
    connect(m_setLevelActionGroup, &QActionGroup::triggered, this, &HierarchyEntry::setLevelTriggered);

    Q_ASSERT(hierarchyLevelNames.size() == (int)HierarchyLevel::EndValue-1);
    m_setLevelMenu = new QMenu(i18n("Set Hierarchy Level"));
    for (int i = 1; i < (int)HierarchyLevel::EndValue; i++)
    {
        QAction* action = new QAction(hierarchyLevelNames[i-1], m_setLevelActionGroup);
        action->setCheckable(true);
        m_setLevelMenu->addAction(action);
    }

    updateFonts(true);
}

HierarchyEntry::~HierarchyEntry()
{
    m_setLevelMenu->deleteLater();
}

void HierarchyEntry::populateMenu(QMenu* menu, QPointF pos)
{
    menu->addMenu(m_setLevelMenu);
    //menu->addSeparator();
    WorksheetEntry::populateMenu(menu, pos);
}

bool HierarchyEntry::isEmpty()
{
    return m_textItem->document()->isEmpty();
}

int HierarchyEntry::type() const
{
    return Type;
}

bool HierarchyEntry::acceptRichText()
{
    return false;
}

bool HierarchyEntry::focusEntry(int pos, qreal xCoord)
{
    if (aboutToBeRemoved())
        return false;
    m_textItem->setFocusAt(pos, xCoord);
    return true;
}


void HierarchyEntry::setContent(const QString& content)
{
    m_textItem->setPlainText(content);
}

void HierarchyEntry::setContent(const QDomElement& content, const KZip& file)
{
    Q_UNUSED(file);
    if(content.firstChildElement(QLatin1String("body")).isNull())
        return;

    m_textItem->setPlainText(content.firstChildElement(QLatin1String("body")).text());

    const QDomElement& subentriesMainElem = content.firstChildElement(QLatin1String("HidedSubentries"));
    if (!subentriesMainElem.isNull())
    {
        m_controlElement.isCollapsable = true;
        m_controlElement.isCollapsed = true;

        const QDomNodeList& entries = subentriesMainElem.childNodes();

        WorksheetEntry* tail = nullptr;
        for (int i = 0; i < entries.size(); i++)
        {
            const QDomElement& entryElem = entries.at(i).toElement();
            int type = Worksheet::typeForTagName(entryElem.tagName());
            Q_ASSERT(type != 0);

            WorksheetEntry* entry = WorksheetEntry::create(type, worksheet());
            entry->setContent(entryElem, file);
            entry->hide();

            // set m_hidedSubentries to head element
            if (!m_hidedSubentries)
                m_hidedSubentries = entry;

            if (tail)
            {
                entry->setPrevious(tail);
                tail->setNext(entry);
                tail = entry;
            }
            else
            {
                entry->setPrevious(nullptr);
                tail = entry;
            }
        }
    }

    m_depth = (HierarchyLevel)content.attribute(QLatin1String("level")).toInt();
    m_hierarchyNumber = content.attribute(QLatin1String("level-number")).toInt();

    updateFonts(true);
}

void HierarchyEntry::setContentFromJupyter(const QJsonObject& cell)
{
    if (Cantor::JupyterUtils::isMarkdownCell(cell))
    {
        QJsonObject cantorMetadata = Cantor::JupyterUtils::getCantorMetadata(cell);
        m_textItem->setPlainText(cantorMetadata.value(QLatin1String("hierarchy_entry_content")).toString());

        m_depth = (HierarchyLevel)cantorMetadata.value(QLatin1String("level")).toInt();
        m_hierarchyNumber= cantorMetadata.value(QLatin1String("level-number")).toInt();

        updateFonts(true);
    }
}

QJsonValue HierarchyEntry::toJupyterJson()
{
    QTextDocument* doc = m_textItem->document();

    QJsonObject metadata(jupyterMetadata());

    QString entryData;
    QString entryType;

    entryType = QLatin1String("markdown");

    // Add raw text of entry to metadata, for situation when
    // Cantor opens .ipynb converted from our .cws format
    QJsonObject cantorMetadata;

    if (Settings::storeTextEntryFormatting())
    {
        entryData = doc->toPlainText();

        cantorMetadata.insert(QLatin1String("hierarchy_entry_content"), entryData);
    }
    else
        entryData = doc->toPlainText();

    cantorMetadata.insert(QLatin1String("level"), (int)m_depth);
    cantorMetadata.insert(QLatin1String("level-number"), m_hierarchyNumber);

    // Don't store subentriesMainElem, because actually too complex
    // Maybe this is a place for future work

    metadata.insert(Cantor::JupyterUtils::cantorMetadataKey, cantorMetadata);

    QJsonObject entry;
    entry.insert(QLatin1String("cell_type"), entryType);
    entry.insert(QLatin1String("metadata"), metadata);
    Cantor::JupyterUtils::setSource(entry, entryData);

    return entry;
}

bool HierarchyEntry::isConvertableToHierarchyEntry(const QJsonObject& cell)
{
    if (!Cantor::JupyterUtils::isMarkdownCell(cell))
        return false;

    QJsonObject cantorMetadata = Cantor::JupyterUtils::getCantorMetadata(cell);
    const QJsonValue& textContentValue = cantorMetadata.value(QLatin1String("hierarchy_entry_content"));

    if (!textContentValue.isString())
        return false;

    const QString& textContent = textContentValue.toString();
    const QString& source = Cantor::JupyterUtils::getSource(cell);

    return textContent == source;
}

QDomElement HierarchyEntry::toXml(QDomDocument& doc, KZip* archive)
{
    Q_UNUSED(archive);
    QDomElement el = doc.createElement(QLatin1String("Hierarchy"));

    QDomElement textBodyEl = doc.createElement(QLatin1String("body"));
    const QString& text = m_textItem->document()->toPlainText();
    textBodyEl.appendChild(doc.createTextNode(text));
    el.appendChild(textBodyEl);

    if(m_hidedSubentries)
    {
        QDomElement entriesElem = doc.createElement(QLatin1String("HidedSubentries"));
        for (WorksheetEntry* entry = m_hidedSubentries; entry; entry = entry->next())
            entriesElem.appendChild(entry->toXml(doc, archive));
        el.appendChild(entriesElem);
    }

    el.setAttribute(QLatin1String("level"), (int)m_depth);
    el.setAttribute(QLatin1String("level-number"), m_hierarchyNumber);

    return el;
}

QString HierarchyEntry::toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq)
{
    Q_UNUSED(commandSep);

    if (commentStartingSeq.isEmpty())
        return QString();

    QString text = m_hierarchyLevelItem->toPlainText() + QLatin1String(" ") + m_textItem->toPlainText();
    if (!commentEndingSeq.isEmpty())
        return commentStartingSeq + text + commentEndingSeq + QLatin1String("\n");
    return commentStartingSeq + text.replace(QLatin1String("\n"), QLatin1String("\n") + commentStartingSeq) + QLatin1String("\n");

}

bool HierarchyEntry::evaluate(EvaluationOption evalOp)
{
    emit hierarhyEntryNameChange(text(), hierarchyText(), ((int)m_depth)-1);
    evaluateNext(evalOp);

    return true;
}

void HierarchyEntry::updateEntry()
{
}

int HierarchyEntry::searchText(const QString& text, const QString& pattern,
                          QTextDocument::FindFlags qt_flags)
{
    Qt::CaseSensitivity caseSensitivity;
    if (qt_flags & QTextDocument::FindCaseSensitively)
        caseSensitivity = Qt::CaseSensitive;
    else
        caseSensitivity = Qt::CaseInsensitive;

    int position;
    if (qt_flags & QTextDocument::FindBackward)
        position = text.lastIndexOf(pattern, -1, caseSensitivity);
    else
        position = text.indexOf(pattern, 0, caseSensitivity);

    return position;
}

WorksheetCursor HierarchyEntry::search(const QString& pattern, unsigned flags,
                                  QTextDocument::FindFlags qt_flags,
                                  const WorksheetCursor& pos)
{
    if (!(flags & WorksheetEntry::SearchText) ||
        (pos.isValid() && pos.entry() != this))
        return WorksheetCursor();

    QTextCursor textCursor = m_textItem->search(pattern, qt_flags, pos);

    if (textCursor.isNull())
        return WorksheetCursor();
    else
        return WorksheetCursor(this, m_textItem, textCursor);
}


void HierarchyEntry::layOutForWidth(qreal entry_zone_x, qreal w, bool force)
{
    if (size().width() == w && m_textItem->pos().x() == entry_zone_x && !force)
        return;

    const qreal margin = worksheet()->isPrinting() ? 0 : RightMargin;

    m_hierarchyLevelItem->setPos(entry_zone_x - m_hierarchyLevelItem->width() - HorizontalSpacing, 0);

    m_textItem->setGeometry(entry_zone_x, 0, w - margin - entry_zone_x);
    setSize(QSizeF(m_textItem->width() + margin + entry_zone_x, std::max(m_textItem->height(), m_hierarchyLevelItem->height()) + VerticalMargin));
}

bool HierarchyEntry::wantToEvaluate()
{
    return false;
}

QString HierarchyEntry::text() const
{
    return m_textItem->toPlainText();
}

QString HierarchyEntry::hierarchyText() const
{
    return m_hierarchyLevelItem->toPlainText();
}

HierarchyEntry::HierarchyLevel HierarchyEntry::level() const
{
    return m_depth;
}

void HierarchyEntry::setLevel(HierarchyEntry::HierarchyLevel depth)
{
    m_depth = depth;
}

int HierarchyEntry::hierarchyNumber() const
{
    return m_hierarchyNumber;
}

void HierarchyEntry::updateControlElementForHierarchy(qreal responsibilityZoneYEnd, int maxHierarchyDepth, bool haveSubElements)
{
    if (!m_hidedSubentries)
        m_controlElement.isCollapsable = haveSubElements;

    qreal controlZoneStart = m_textItem->x() + m_textItem->width() + HorizontalSpacing;
    qreal controlElemenXPos = controlZoneStart + static_cast<double>(maxHierarchyDepth - (int)m_depth + 1) * (ControlElementWidth + ControlElementBorder);

    m_controlElement.setRect(
        controlElemenXPos, 0,
        ControlElementWidth, responsibilityZoneYEnd-y()
    );
    m_controlElement.update();
    update();
}


void HierarchyEntry::updateHierarchyLevel(std::vector<int>& currectNumbers)
{
    HierarchyLevel nextLevel = (HierarchyLevel)(currectNumbers.size()+1);
    if (m_depth >= nextLevel)
    {
        for (int i = (int)nextLevel; i <= (int)m_depth; i++)
            currectNumbers.push_back(1);
        m_hierarchyNumber = 1;
    }
    else
    {
        int idx = (int)m_depth - 1;
        size_t size = currectNumbers.size();
        for (size_t i = (size_t)idx+1; i < size; i++)
            currectNumbers.pop_back();
        currectNumbers[idx] += 1;
        m_hierarchyNumber = currectNumbers[idx];
    }

    QString s;
    Q_ASSERT(currectNumbers.size() != 0);
    s += QString::number(currectNumbers.front());
    for (size_t i = 1U; i < currectNumbers.size(); i++)
        s += QLatin1String(".") + QString::number(currectNumbers[i]);

    qreal previousWidth = m_hierarchyLevelItem->width();
    m_hierarchyLevelItem->setPlainText(s);
    m_hierarchyLevelItem->setPos(m_hierarchyLevelItem->x() - (m_hierarchyLevelItem->width() - previousWidth), 0);
    updateFonts();
}

qreal HierarchyEntry::hierarchyItemWidth()
{
    return m_hierarchyLevelItem->width() + HorizontalSpacing;
}

void HierarchyEntry::setLevelTriggered(QAction* action)
{
    int idx = m_setLevelActionGroup->actions().indexOf(action);
    m_depth = (HierarchyLevel)(idx + 1);

    worksheet()->updateHierarchyLayout();
    worksheet()->updateLayout();
}

void HierarchyEntry::recalculateControlGeometry()
{
    // do nothing, update the control elements will be done in ;;updateControlElementForHierarchy
}

void HierarchyEntry::startDrag(QPointF grabPos)
{
    // We need reset entry cursor manually, because otherwise the entry cursor will be visible on dragable item
    worksheet()->resetEntryCursor();

    QDrag* drag = new QDrag(worksheetView());
    const qreal scale = worksheet()->renderer()->scale();

    QRectF hierarchyBound(boundingRect().x(), boundingRect().y(), boundingRect().width(), m_controlElement.boundingRect().height());
    QSizeF hierarchyZoneSize(size().width(), m_controlElement.boundingRect().height());

    QPixmap pixmap((hierarchyZoneSize*scale).toSize());
    pixmap.fill(QColor(255, 255, 255, 0));

    QPainter painter(&pixmap);
    const QRectF sceneRect = mapRectToScene(hierarchyBound);
    worksheet()->render(&painter, pixmap.rect(), sceneRect);
    painter.end();

    QBitmap mask = pixmap.createMaskFromColor(QColor(255, 255, 255), Qt::MaskInColor);
    pixmap.setMask(mask);

    drag->setPixmap(pixmap);
    if (grabPos.isNull()) {
        const QPointF scenePos = worksheetView()->sceneCursorPos();
        drag->setHotSpot((mapFromScene(scenePos) * scale).toPoint());
    } else {
        drag->setHotSpot((grabPos * scale).toPoint());
    }
    drag->setMimeData(new QMimeData());

    worksheet()->startDragWithHierarchy(this, drag, hierarchyZoneSize);
}

void HierarchyEntry::updateFonts(bool force)
{
    QFont font;
    switch(m_depth)
    {
        case HierarchyLevel::Chapter:
            font = Settings::chapterFontFamily();
            font.setPointSize(Settings::chapterFontSize());
            font.setItalic(Settings::chapterFontItalic());
            font.setBold(Settings::chapterFontBold());
            break;

        case HierarchyLevel::Subchapter:
            font = Settings::subchapterFontFamily();
            font.setPointSize(Settings::subchapterFontSize());
            font.setItalic(Settings::subchapterFontItalic());
            font.setBold(Settings::subchapterFontBold());
            break;

        case HierarchyLevel::Section:
            font = Settings::sectionFontFamily();
            font.setPointSize(Settings::sectionFontSize());
            font.setItalic(Settings::sectionFontItalic());
            font.setBold(Settings::sectionFontBold());
            break;

        case HierarchyLevel::Subsection:
            font = Settings::subsectionFontFamily();
            font.setPointSize(Settings::subsectionFontSize());
            font.setItalic(Settings::subsectionFontItalic());
            font.setBold(Settings::subsectionFontBold());
            break;

        case HierarchyLevel::Paragraph:
            font = Settings::paragraphFontFamily();
            font.setPointSize(Settings::paragraphFontSize());
            font.setItalic(Settings::paragraphFontItalic());
            font.setBold(Settings::paragraphFontBold());
            break;

        case HierarchyLevel::Subparagraph:
            font = Settings::subparagraphFontFamily();
            font.setPointSize(Settings::subparagraphFontSize());
            font.setItalic(Settings::subparagraphFontItalic());
            font.setBold(Settings::subparagraphFontBold());
            break;

        default:
            Q_ASSERT(false);
            break;
    }

    const QFont& currectFont = m_textItem->font();
    bool isSameFont =
           currectFont.family() == font.family()
        && currectFont.pointSize() == font.pointSize()
        && currectFont.bold() == font.bold()
        && currectFont.italic() == font.italic();

    if (force || !isSameFont)
    {
        m_hierarchyLevelItem->setFont(font);
        m_hierarchyLevelItem->testSize();

        m_textItem->setFont(font);
        // And update current text of item
        QTextCursor cursor = m_textItem->textCursor();
        cursor.select(QTextCursor::Document);
        QTextCharFormat format = cursor.charFormat();
        format.setFont(font);
        cursor.setCharFormat(format);
        m_textItem->testSize();

        // Recalculate size (because it can changed due font changes) and update worksheet layout
        recalculateSize();
        worksheet()->updateEntrySize(this);
    }
}


void HierarchyEntry::handleControlElementDoubleClick()
{
    qDebug() << "HierarchyEntry::handleControlElementDoubleClick";
    if (m_controlElement.isCollapsed)
    {
        worksheet()->insertSubentriesForHierarchy(this, m_hidedSubentries);
        m_controlElement.isCollapsed = false;
    }
    else
    {
        m_hidedSubentries = worksheet()->cutSubentriesForHierarchy(this);
        m_controlElement.isCollapsed = true;
    }

    m_controlElement.update();

    worksheet()->updateLayout();
    worksheet()->updateHierarchyLayout();
}

void HierarchyEntry::updateAfterSettingsChanges()
{
    updateFonts();
}
