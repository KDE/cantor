/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2020 Sirgienko Nikita <warquark@gmail.com>
*/

#include "horizontalruleentry.h"

#include <QPropertyAnimation>
#include <QJsonObject>
#include <QApplication>

#include <QDebug>

#include <KLocalizedString>

#include <jupyterutils.h>

const qreal HorizontalRuleEntry::LineVerticalMargin = 10;

const QString HorizontalRuleEntry::styleNames[] = {i18n("Solid Line Style"), i18n("Dash Line Style"), i18n("Dot Line Style"), i18n("Dash Dot Line Style"), i18n("Dash Dot Dot Line Style")};
const Qt::PenStyle HorizontalRuleEntry::styles[] = {Qt::SolidLine, Qt::DashLine, Qt::DotLine, Qt::DashDotLine, Qt::DashDotDotLine};

HorizontalRuleEntry::HorizontalRuleEntry(Worksheet* worksheet)
    : WorksheetEntry(worksheet), m_type(LineType::Medium), m_color(QApplication::palette().color(QPalette::Text)), m_entry_zone_offset_x(0), m_width(0), m_style(Qt::SolidLine),
      m_menusInitialized(false), m_lineTypeActionGroup(nullptr), m_lineTypeMenu(nullptr), m_lineColorCustom(false), m_lineColorActionGroup(nullptr), m_lineColorMenu(nullptr),
      m_lineStyleActionGroup(nullptr), m_lineStyleMenu(nullptr)
{
}

HorizontalRuleEntry::~HorizontalRuleEntry()
{
    if (m_menusInitialized)
    {
        m_lineColorActionGroup->deleteLater();
        m_lineColorMenu->deleteLater();
        m_lineTypeActionGroup->deleteLater();
        m_lineTypeMenu->deleteLater();
        m_lineStyleActionGroup->deleteLater();
        m_lineStyleMenu->deleteLater();
    }
}


int HorizontalRuleEntry::type() const
{
    return Type;
}

void HorizontalRuleEntry::setLineType(HorizontalRuleEntry::LineType type)
{
    m_type = type;

    setSize(QSizeF(m_width, lineWidth(m_type) + 2*LineVerticalMargin));
}

int HorizontalRuleEntry::lineWidth(HorizontalRuleEntry::LineType type)
{
    return type == LineType::Thick ? 4 : ((int)type) + 1;
}

void HorizontalRuleEntry::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setPen(QPen(m_color, lineWidth(m_type), m_style));

    const qreal margin = worksheet()->isPrinting() ? 0 : RightMargin;

    painter->drawLine(m_entry_zone_offset_x, LineVerticalMargin, m_width - margin, LineVerticalMargin);
}

bool HorizontalRuleEntry::isEmpty()
{
    return true;
}

bool HorizontalRuleEntry::acceptRichText()
{
    return false;
}

void HorizontalRuleEntry::setContent(const QString&)
{
}

void HorizontalRuleEntry::setContent(const QDomElement& content, const KZip& archive)
{
    Q_UNUSED(archive);

    m_type = (LineType)(content.attribute(QLatin1String("thickness"), QString::number((int)LineType::Medium)).toInt());
    m_style = (Qt::PenStyle)(content.attribute(QLatin1String("style"), QString::number((int)Qt::SolidLine)).toInt());

    QDomElement backgroundElem = content.firstChildElement(QLatin1String("lineColor"));
    if (!backgroundElem.isNull())
    {
        m_color.setRed(backgroundElem.attribute(QLatin1String("red")).toInt());
        m_color.setGreen(backgroundElem.attribute(QLatin1String("green")).toInt());
        m_color.setBlue(backgroundElem.attribute(QLatin1String("blue")).toInt());
        m_lineColorCustom = true;
    }
}

void HorizontalRuleEntry::setContentFromJupyter(const QJsonObject& cell)
{
    QJsonObject cantorMetadata = Cantor::JupyterUtils::getCantorMetadata(cell);
    QJsonValue typeValue = cantorMetadata.value(QLatin1String("type"));
    if (typeValue.isDouble())
        setLineType(static_cast<LineType>(static_cast<int>(typeValue.toDouble())));

    QJsonValue styleValue = cantorMetadata.value(QLatin1String("style"));
    if (styleValue.isDouble())
        m_style = static_cast<Qt::PenStyle>(static_cast<int>(styleValue.toDouble()));

    QJsonValue colorValue = cantorMetadata.value(QLatin1String("lineColor"));
    if (colorValue.isObject())
    {
        m_color.setRed(colorValue.toObject().value(QLatin1String("red")).toInt());
        m_color.setGreen(colorValue.toObject().value(QLatin1String("green")).toInt());
        m_color.setBlue(colorValue.toObject().value(QLatin1String("blue")).toInt());
        m_lineColorCustom = true;
    }

    setJupyterMetadata(Cantor::JupyterUtils::getMetadata(cell));
}

QJsonValue HorizontalRuleEntry::toJupyterJson()
{
    QJsonObject entry;

    entry.insert(QLatin1String("cell_type"), QLatin1String("markdown"));
    QJsonObject metadata(jupyterMetadata());

    QJsonObject cantor;
    cantor.insert(QLatin1String("type"), m_type);
    cantor.insert(QLatin1String("style"), m_style);

    if (m_lineColorCustom)
    {
        QJsonObject color;
        color.insert(QLatin1String("red"), m_color.red());
        color.insert(QLatin1String("green"), m_color.green());
        color.insert(QLatin1String("blue"), m_color.blue());
        cantor.insert(QLatin1String("lineColor"), color);
    }

    metadata.insert(Cantor::JupyterUtils::cantorMetadataKey, cantor);

    entry.insert(Cantor::JupyterUtils::metadataKey, metadata);

    Cantor::JupyterUtils::setSource(entry, QLatin1String("----"));
    return entry;
}


QDomElement HorizontalRuleEntry::toXml(QDomDocument& doc, KZip* archive)
{
    Q_UNUSED(archive);

    QDomElement el = doc.createElement(QLatin1String("HorizontalRule"));
    el.setAttribute(QLatin1String("thickness"), (int)m_type);
    el.setAttribute(QLatin1String("style"), (int)m_style);

    if (m_lineColorCustom)
    {
        QColor backgroundColor = m_color;
        QDomElement colorElem = doc.createElement( QLatin1String("lineColor") );
        colorElem.setAttribute(QLatin1String("red"), QString::number(backgroundColor.red()));
        colorElem.setAttribute(QLatin1String("green"), QString::number(backgroundColor.green()));
        colorElem.setAttribute(QLatin1String("blue"), QString::number(backgroundColor.blue()));
        el.appendChild(colorElem);
    }

    return el;
}

QString HorizontalRuleEntry::toPlain(const QString&, const QString&, const QString&){
    return QString();
}

void HorizontalRuleEntry::interruptEvaluation()
{
    return;
}

void HorizontalRuleEntry::layOutForWidth(qreal entry_zone_x, qreal w, bool force)
{
    Q_UNUSED(force);

    m_entry_zone_offset_x = entry_zone_x;
    m_width = w;

    setSize(QSizeF(w, lineWidth(m_type) + 2*LineVerticalMargin));
}

bool HorizontalRuleEntry::evaluate(EvaluationOption evalOp)
{
    evaluateNext(evalOp);
    return true;
}

void HorizontalRuleEntry::updateEntry()
{
}

bool HorizontalRuleEntry::wantToEvaluate()
{
    return false;
}

void HorizontalRuleEntry::changeSize(QSizeF s)
{
    if (!worksheet()->animationsEnabled()) {
        setSize(s);
        worksheet()->updateEntrySize(this);
        return;
    }
    if (aboutToBeRemoved())
        return;

    if (animationActive())
        endAnimation();

    QPropertyAnimation* sizeAn = sizeChangeAnimation(s);

    sizeAn->setEasingCurve(QEasingCurve::InOutQuad);
    sizeAn->start(QAbstractAnimation::DeleteWhenStopped);
}

void HorizontalRuleEntry::populateMenu(QMenu* menu, QPointF pos)
{
    if (!m_menusInitialized)
    {
        initMenus();
        m_menusInitialized = true;
    }

    menu->addMenu(m_lineTypeMenu);
    menu->addMenu(m_lineColorMenu);
    menu->addMenu(m_lineStyleMenu);
    WorksheetEntry::populateMenu(menu, pos);
}

void HorizontalRuleEntry::lineTypeChanged(QAction* action)
{
    int index = m_lineTypeActionGroup->actions().indexOf(action);
    setLineType((LineType)(index % LineType::Count));
}

bool HorizontalRuleEntry::isConvertableToHorizontalRuleEntry(const QJsonObject& cell)
{
    if (!Cantor::JupyterUtils::isMarkdownCell(cell))
        return false;

    const QString& trimmedSource = Cantor::JupyterUtils::getSource(cell).trimmed();

    int sourceLength = trimmedSource.length();
    if (sourceLength < 3)
        return false;

    int hyphensCount = trimmedSource.count(QLatin1Char('-'));
    int asteriksCount = trimmedSource.count(QLatin1Char('*'));
    int underscoreCount = trimmedSource.count(QLatin1Char('_'));

    return sourceLength == hyphensCount || sourceLength == asteriksCount || sourceLength == underscoreCount;
}

void HorizontalRuleEntry::lineColorChanged(QAction* action) {
    int index = m_lineColorActionGroup->actions().indexOf(action);
    if (index == -1 || index>=colorsCount)
        index = 0;

    if (index == 0)
    {
        m_color = QApplication::palette().color(QPalette::Text);
        m_lineColorCustom = false;
    }
    else
    {
        m_color = colors[index-1];
        m_lineColorCustom = true;
    }
    update();
}

void HorizontalRuleEntry::lineStyleChanged(QAction* action)
{
    int index = m_lineStyleActionGroup->actions().indexOf(action);
    m_style = styles[index];
    update();
}


void HorizontalRuleEntry::initMenus()
{
    m_lineTypeActionGroup = new QActionGroup(this);
    m_lineTypeActionGroup->setExclusive(true);
    connect(m_lineTypeActionGroup, &QActionGroup::triggered, this, &HorizontalRuleEntry::lineTypeChanged);

    m_lineTypeMenu = new QMenu(i18n("Line Thickness"));

    QAction* action = new QAction(i18n("Thin"), m_lineTypeActionGroup);
    action->setCheckable(true);
    m_lineTypeMenu->addAction(action);

    action = new QAction(i18n("Medium"), m_lineTypeActionGroup);
    action->setCheckable(true);
    m_lineTypeMenu->addAction(action);

    action = new QAction(i18n("Thick"), m_lineTypeActionGroup);
    action->setCheckable(true);
    m_lineTypeMenu->addAction(action);

    // Set default menu value
    m_lineTypeActionGroup->actions()[(int)m_type]->setChecked(true);



    m_lineColorActionGroup = new QActionGroup(this);
    m_lineColorActionGroup->setExclusive(true);
    connect(m_lineColorActionGroup, &QActionGroup::triggered, this, &HorizontalRuleEntry::lineColorChanged);

    m_lineColorMenu = new QMenu(i18n("Line Color"));
    m_lineColorMenu->setIcon(QIcon::fromTheme(QLatin1String("format-fill-color")));

    QPixmap pix(16,16);
    QPainter p(&pix);

    // Create default action
    p.fillRect(pix.rect(), QApplication::palette().color(QPalette::Text));
    action = new QAction(QIcon(pix), i18n("Default"), m_lineColorActionGroup);
    action->setCheckable(true);
    m_lineColorMenu->addAction(action);
    if (!m_lineColorCustom)
        action->setChecked(true);


    for (int i=0; i<colorsCount; ++i) {
        p.fillRect(pix.rect(), colors[i]);
        action = new QAction(QIcon(pix), colorNames[i], m_lineColorActionGroup);
        action->setCheckable(true);
        m_lineColorMenu->addAction(action);

        if (m_lineColorCustom && m_color == colors[i])
            action->setChecked(true);
    }



    m_lineStyleActionGroup = new QActionGroup(this);
    m_lineStyleActionGroup->setExclusive(true);
    connect(m_lineStyleActionGroup, &QActionGroup::triggered, this, &HorizontalRuleEntry::lineStyleChanged);

    m_lineStyleMenu = new QMenu(i18n("Line Style"));

    for (unsigned int i = 0; i < styleCount; i++)
    {
        action = new QAction(styleNames[i], m_lineStyleActionGroup);
        action->setCheckable(true);
        m_lineStyleMenu->addAction(action);
        if (styles[i] == m_style)
            action->setChecked(true);
    }
}
