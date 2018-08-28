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
    Copyright (C) 2018 Alexander Semke <alexander.semke@web.de>
 */

#include "commandentry.h"
#include "resultitem.h"
#include "loadedexpression.h"
#include "lib/result.h"
#include "lib/helpresult.h"
#include "lib/completionobject.h"
#include "lib/syntaxhelpobject.h"
#include "lib/session.h"

#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QFontDialog>
#include <QTimer>
#include <QToolTip>

#include <KLocalizedString>
#include <KColorScheme>

const QString CommandEntry::Prompt=QLatin1String(">>> ");
const double CommandEntry::HorizontalSpacing = 4;
const double CommandEntry::VerticalSpacing = 4;

static const int colorsCount = 26;
static QColor colors[colorsCount] = {QColor(255,255,255), QColor(0,0,0),
							QColor(192,0,0), QColor(255,0,0), QColor(255,192,192), //red
							QColor(0,192,0), QColor(0,255,0), QColor(192,255,192), //green
							QColor(0,0,192), QColor(0,0,255), QColor(192,192,255), //blue
							QColor(192,192,0), QColor(255,255,0), QColor(255,255,192), //yellow
							QColor(0,192,192), QColor(0,255,255), QColor(192,255,255), //cyan
							QColor(192,0,192), QColor(255,0,255), QColor(255,192,255), //magenta
							QColor(192,88,0), QColor(255,128,0), QColor(255,168,88), //orange
							QColor(128,128,128), QColor(160,160,160), QColor(195,195,195) //grey
							};


CommandEntry::CommandEntry(Worksheet* worksheet) : WorksheetEntry(worksheet),
    m_promptItem(new WorksheetTextItem(this, Qt::NoTextInteraction)),
    m_commandItem(new WorksheetTextItem(this, Qt::TextEditorInteraction)),
    m_errorItem(nullptr),
    m_expression(nullptr),
    m_completionObject(nullptr),
    m_syntaxHelpObject(nullptr),
    m_menusInitialized(false),
    m_backgroundColorActionGroup(nullptr),
    m_backgroundColorMenu(nullptr),
    m_textColorActionGroup(nullptr),
    m_textColorMenu(nullptr),
    m_fontMenu(nullptr)
{

    m_promptItem->setPlainText(Prompt);
    m_promptItem->setItemDragable(true);
    m_commandItem->enableCompletion(true);

    KColorScheme scheme = KColorScheme(QPalette::Normal, KColorScheme::View);
    m_commandItem->setBackgroundColor(scheme.background(KColorScheme::AlternateBackground).color());

    m_promtItemTimer = new QTimer(this);
    connect(m_promtItemTimer, &QTimer::timeout, this, &CommandEntry::animatePromptItem);

    connect(m_commandItem, &WorksheetTextItem::tabPressed, this, &CommandEntry::showCompletion);
    connect(m_commandItem, &WorksheetTextItem::backtabPressed, this, &CommandEntry::selectPreviousCompletion);
    connect(m_commandItem, &WorksheetTextItem::applyCompletion, this, &CommandEntry::applySelectedCompletion);
    connect(m_commandItem, SIGNAL(execute()), this, SLOT(evaluate()));
    connect(m_commandItem, &WorksheetTextItem::moveToPrevious, this, &CommandEntry::moveToPreviousItem);
    connect(m_commandItem, &WorksheetTextItem::moveToNext, this, &CommandEntry::moveToNextItem);
    connect(m_commandItem, SIGNAL(receivedFocus(WorksheetTextItem*)), worksheet, SLOT(highlightItem(WorksheetTextItem*)));
    connect(m_promptItem, &WorksheetTextItem::drag, this, &CommandEntry::startDrag);
    connect(worksheet, SIGNAL(updatePrompt()), this, SLOT(updatePrompt()));
}

CommandEntry::~CommandEntry()
{
    if (m_completionBox)
        m_completionBox->deleteLater();
}

int CommandEntry::type() const
{
    return Type;
}

void CommandEntry::initMenus() {
    //background color
	const QString colorNames[colorsCount] = {i18n("White"), i18n("Black"),
							i18n("Dark Red"), i18n("Red"), i18n("Light Red"),
							i18n("Dark Green"), i18n("Green"), i18n("Light Green"),
							i18n("Dark Blue"), i18n("Blue"), i18n("Light Blue"),
							i18n("Dark Yellow"), i18n("Yellow"), i18n("Light Yellow"),
							i18n("Dark Cyan"), i18n("Cyan"), i18n("Light Cyan"),
							i18n("Dark Magenta"), i18n("Magenta"), i18n("Light Magenta"),
							i18n("Dark Orange"), i18n("Orange"), i18n("Light Orange"),
							i18n("Dark Grey"), i18n("Grey"), i18n("Light Grey")
							};

    //background color
    m_backgroundColorActionGroup = new QActionGroup(this);
	m_backgroundColorActionGroup->setExclusive(true);
	connect(m_backgroundColorActionGroup, &QActionGroup::triggered, this, &CommandEntry::backgroundColorChanged);

    m_backgroundColorMenu = new QMenu(i18n("Background Color"));
    m_backgroundColorMenu->setIcon(QIcon::fromTheme(QLatin1String("format-fill-color")));

	QPixmap pix(16,16);
	QPainter p(&pix);
	for (int i=0; i<colorsCount; ++i) {
		p.fillRect(pix.rect(), colors[i]);
		QAction* action = new QAction(QIcon(pix), colorNames[i], m_backgroundColorActionGroup);
		action->setCheckable(true);
		m_backgroundColorMenu->addAction(action);
	}

	//text color
    m_textColorActionGroup = new QActionGroup(this);
	m_textColorActionGroup->setExclusive(true);
	connect(m_textColorActionGroup, &QActionGroup::triggered, this, &CommandEntry::textColorChanged);

    m_textColorMenu = new QMenu(i18n("Text Color"));
    m_textColorMenu->setIcon(QIcon::fromTheme(QLatin1String("format-text-color")));

	for (int i=0; i<colorsCount; ++i) {
		p.fillRect(pix.rect(), colors[i]);
		QAction* action = new QAction(QIcon(pix), colorNames[i], m_textColorActionGroup);
		action->setCheckable(true);
		m_textColorMenu->addAction(action);
	}

	//font
	m_fontMenu = new QMenu(i18n("Font"));
    m_fontMenu->setIcon(QIcon::fromTheme(QLatin1String("preferences-desktop-font")));

    QAction* action = new QAction(QIcon::fromTheme(QLatin1String("format-text-bold")), i18n("Bold"));
    action->setCheckable(true);
    connect(action, &QAction::triggered, this, &CommandEntry::fontBoldTriggered);
    m_fontMenu->addAction(action);

    action = new QAction(QIcon::fromTheme(QLatin1String("format-text-italic")), i18n("Italic"));
    action->setCheckable(true);
    connect(action, &QAction::triggered, this, &CommandEntry::fontItalicTriggered);
    m_fontMenu->addAction(action);
    m_fontMenu->addSeparator();

    action = new QAction(QIcon::fromTheme(QLatin1String("format-font-size-less")), i18n("Increase Size"));
    connect(action, &QAction::triggered, this, &CommandEntry::fontIncreaseTriggered);
    m_fontMenu->addAction(action);

    action = new QAction(QIcon::fromTheme(QLatin1String("format-font-size-more")), i18n("Decrease Size"));
    connect(action, &QAction::triggered, this, &CommandEntry::fontDecreaseTriggered);
    m_fontMenu->addAction(action);
    m_fontMenu->addSeparator();

    action = new QAction(QIcon::fromTheme(QLatin1String("preferences-desktop-font")), i18n("Select"));
    connect(action, &QAction::triggered, this, &CommandEntry::fontSelectTriggered);
    m_fontMenu->addAction(action);

    m_menusInitialized = true;
}

void CommandEntry::backgroundColorChanged(QAction* action) {
	int index = m_backgroundColorActionGroup->actions().indexOf(action);
	if (index == -1 || index>=colorsCount)
		index = 0;

    m_commandItem->setBackgroundColor(colors[index]);
}

void CommandEntry::textColorChanged(QAction* action) {
	int index = m_textColorActionGroup->actions().indexOf(action);
	if (index == -1 || index>=colorsCount)
		index = 0;

    m_commandItem->setDefaultTextColor(colors[index]);
}

void CommandEntry::fontBoldTriggered()
{
    QAction* action = static_cast<QAction*>(QObject::sender());
    QFont font = m_commandItem->font();
    font.setBold(action->isChecked());
    m_commandItem->setFont(font);
}

void CommandEntry::fontItalicTriggered()
{
    QAction* action = static_cast<QAction*>(QObject::sender());
    QFont font = m_commandItem->font();
    font.setItalic(action->isChecked());
    m_commandItem->setFont(font);
}

void CommandEntry::fontIncreaseTriggered()
{
    QFont font = m_commandItem->font();
    const int currentSize = font.pointSize();
    QFontDatabase fdb;
    QList<int> sizes = fdb.pointSizes(font.family(), font.styleName());

    for (int i = 0; i < sizes.size(); ++i)
    {
        const int size = sizes.at(i);
        if (currentSize == size)
        {
            if (i + 1 < sizes.size())
            {
                font.setPointSize(sizes.at(i+1));
                m_commandItem->setFont(font);
            }

            break;
        }
    }
}

void CommandEntry::fontDecreaseTriggered()
{
    QFont font = m_commandItem->font();
    const int currentSize = font.pointSize();
    QFontDatabase fdb;
    QList<int> sizes = fdb.pointSizes(font.family(), font.styleName());

    for (int i = 0; i < sizes.size(); ++i)
    {
        const int size = sizes.at(i);
        if (currentSize == size)
        {
            if (i - 1 >= 0)
            {
                font.setPointSize(sizes.at(i-1));
                m_commandItem->setFont(font);
            }

            break;
        }
    }
}

void CommandEntry::fontSelectTriggered()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, m_commandItem->font(), nullptr);

    if (ok)
        m_commandItem->setFont(font);
}

void CommandEntry::populateMenu(QMenu* menu, QPointF pos)
{
    if (!m_menusInitialized)
        initMenus();

    menu->addMenu(m_backgroundColorMenu);
    menu->addMenu(m_textColorMenu);
    menu->addMenu(m_fontMenu);
    menu->addSeparator();
    WorksheetEntry::populateMenu(menu, pos);
}

void CommandEntry::moveToNextItem(int pos, qreal x)
{
    WorksheetTextItem* item = qobject_cast<WorksheetTextItem*>(sender());

    if (!item)
        return;

    if (item == m_commandItem) {
        if (m_informationItems.isEmpty() ||
            !currentInformationItem()->isEditable())
            moveToNextEntry(pos, x);
        else
            currentInformationItem()->setFocusAt(pos, x);
    } else if (item == currentInformationItem()) {
        moveToNextEntry(pos, x);
    }
}

void CommandEntry::moveToPreviousItem(int pos, qreal x)
{
    WorksheetTextItem* item = qobject_cast<WorksheetTextItem*>(sender());

    if (!item)
        return;

    if (item == m_commandItem || item == nullptr) {
        moveToPreviousEntry(pos, x);
    } else if (item == currentInformationItem()) {
        m_commandItem->setFocusAt(pos, x);
    }
}

QString CommandEntry::command()
{
    QString cmd = m_commandItem->toPlainText();
    cmd.replace(QChar::ParagraphSeparator, QLatin1Char('\n')); //Replace the U+2029 paragraph break by a Normal Newline
    cmd.replace(QChar::LineSeparator, QLatin1Char('\n')); //Replace the line break by a Normal Newline
    return cmd;
}

void CommandEntry::setExpression(Cantor::Expression* expr)
{
    /*
    if ( m_expression ) {
        if (m_expression->status() == Cantor::Expression::Computing) {
            qDebug() << "OLD EXPRESSION STILL ACTIVE";
            m_expression->interrupt();
        }
        m_expression->deleteLater();
        }*/

    // Delete any previous error
    if(m_errorItem)
    {
        m_errorItem->deleteLater();
        m_errorItem = nullptr;
    }

    foreach(WorksheetTextItem* item, m_informationItems)
    {
        item->deleteLater();
    }
    m_informationItems.clear();

    m_expression = nullptr;
    // Delete any previous result
    removeResult();

    m_expression=expr;

    connect(expr, SIGNAL(gotResult()), this, SLOT(updateEntry()));
    connect(expr, SIGNAL(idChanged()), this, SLOT(updatePrompt()));
    connect(expr, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SLOT(expressionChangedStatus(Cantor::Expression::Status)));
    connect(expr, SIGNAL(needsAdditionalInformation(const QString&)), this, SLOT(showAdditionalInformationPrompt(const QString&)));
    connect(expr, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SLOT(updatePrompt()));

    updatePrompt();

    if(expr->result())
    {
        worksheet()->gotResult(expr);
        updateEntry();
    }

    expressionChangedStatus(expr->status());
}

Cantor::Expression* CommandEntry::expression()
{
    return m_expression;
}

bool CommandEntry::acceptRichText()
{
    return false;
}

void CommandEntry::setContent(const QString& content)
{
    m_commandItem->setPlainText(content);
}

void CommandEntry::setContent(const QDomElement& content, const KZip& file)
{
    m_commandItem->setPlainText(content.firstChildElement(QLatin1String("Command")).text());

    LoadedExpression* expr=new LoadedExpression( worksheet()->session() );
    expr->loadFromXml(content, file);

    //background color
    QDomElement backgroundElem = content.firstChildElement(QLatin1String("Background"));
    if (!backgroundElem.isNull())
    {
        QColor color;
        color.setRed(backgroundElem.attribute(QLatin1String("red")).toInt());
        color.setGreen(backgroundElem.attribute(QLatin1String("green")).toInt());
        color.setBlue(backgroundElem.attribute(QLatin1String("blue")).toInt());
        m_commandItem->setBackgroundColor(color);
    }

    //text properties
    QDomElement textElem = content.firstChildElement(QLatin1String("Text"));
    if (!textElem.isNull())
    {
        //text color
        QDomElement colorElem = textElem.firstChildElement(QLatin1String("Color"));
        QColor color;
        color.setRed(colorElem.attribute(QLatin1String("red")).toInt());
        color.setGreen(colorElem.attribute(QLatin1String("green")).toInt());
        color.setBlue(colorElem.attribute(QLatin1String("blue")).toInt());
        m_commandItem->setDefaultTextColor(color);

        //font properties
        QDomElement fontElem = textElem.firstChildElement(QLatin1String("Font"));
        QFont font;
        font.setFamily(fontElem.attribute(QLatin1String("family")));
        font.setPointSize(fontElem.attribute(QLatin1String("pointSize")).toInt());
        font.setWeight(fontElem.attribute(QLatin1String("weight")).toInt());
        font.setItalic(fontElem.attribute(QLatin1String("italic")).toInt());
        m_commandItem->setFont(font);
    }

    setExpression(expr);
}

void CommandEntry::showCompletion()
{
    const QString line = currentLine();

    if(!worksheet()->completionEnabled() || line.trimmed().isEmpty())
    {
        if (m_commandItem->hasFocus())
            m_commandItem->insertTab();
        return;
    } else if (isShowingCompletionPopup()) {
        QString comp = m_completionObject->completion();
        qDebug() << "command" << m_completionObject->command();
        qDebug() << "completion" << comp;
        if (comp != m_completionObject->command()
            || !m_completionObject->hasMultipleMatches()) {
            if (m_completionObject->hasMultipleMatches()) {
                completeCommandTo(comp, PreliminaryCompletion);
            } else {
                completeCommandTo(comp, FinalCompletion);
                m_completionBox->hide();
            }
        } else {
            m_completionBox->down();
        }
    } else {
        int p = m_commandItem->textCursor().positionInBlock();
        Cantor::CompletionObject* tco=worksheet()->session()->completionFor(line, p);
        if(tco)
            setCompletion(tco);
    }
}

void CommandEntry::selectPreviousCompletion()
{
    if (isShowingCompletionPopup())
        m_completionBox->up();
}

QString CommandEntry::toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq)
{
    Q_UNUSED(commentStartingSeq);
    Q_UNUSED(commentEndingSeq);

    if (command().isEmpty())
        return QString();
    return command() + commandSep;
}

QDomElement CommandEntry::toXml(QDomDocument& doc, KZip* archive)
{
    QDomElement exprElem = doc.createElement( QLatin1String("Expression") );
    QDomElement cmdElem = doc.createElement( QLatin1String("Command") );
    cmdElem.appendChild(doc.createTextNode( command() ));
    exprElem.appendChild(cmdElem);

    //save the result if available
    if (expression())
    {
        if (expression()->result())
        {
            QDomElement resultElem = expression()->result()->toXml(doc);
            exprElem.appendChild(resultElem);

            if (archive)
                expression()->result()->saveAdditionalData(archive);
        }
    }

    //save the background color if it differs from the default one
    const QColor& backgroundColor = m_commandItem->backgroundColor();
    KColorScheme scheme = KColorScheme(QPalette::Normal, KColorScheme::View);
    if (backgroundColor != scheme.background(KColorScheme::AlternateBackground).color())
    {
        QDomElement colorElem = doc.createElement( QLatin1String("Background") );
        colorElem.setAttribute(QLatin1String("red"), QString::number(backgroundColor.red()));
        colorElem.setAttribute(QLatin1String("green"), QString::number(backgroundColor.green()));
        colorElem.setAttribute(QLatin1String("blue"), QString::number(backgroundColor.blue()));
        exprElem.appendChild(colorElem);
    }

    //save the text properties if they differ from default values
    const QFont& font = m_commandItem->font();
    if (font != QFontDatabase::systemFont(QFontDatabase::FixedFont))
    {
        QDomElement textElem = doc.createElement(QLatin1String("Text"));

        //font properties
        QDomElement fontElem = doc.createElement(QLatin1String("Font"));
        fontElem.setAttribute(QLatin1String("family"), font.family());
        fontElem.setAttribute(QLatin1String("pointSize"), QString::number(font.pointSize()));
        fontElem.setAttribute(QLatin1String("weight"), QString::number(font.weight()));
        fontElem.setAttribute(QLatin1String("italic"), QString::number(font.italic()));
        textElem.appendChild(fontElem);

        //text color
        const QColor& textColor = m_commandItem->defaultTextColor();
        QDomElement colorElem = doc.createElement( QLatin1String("Color") );
        colorElem.setAttribute(QLatin1String("red"), QString::number(textColor.red()));
        colorElem.setAttribute(QLatin1String("green"), QString::number(textColor.green()));
        colorElem.setAttribute(QLatin1String("blue"), QString::number(textColor.blue()));
        textElem.appendChild(colorElem);

        exprElem.appendChild(textElem);
    }

    return exprElem;
}

QString CommandEntry::currentLine()
{
    if (!m_commandItem->hasFocus())
        return QString();

    QTextBlock block = m_commandItem->textCursor().block();
    return block.text();
}

bool CommandEntry::evaluateCurrentItem()
{
    // we can't use m_commandItem->hasFocus() here, because
    // that doesn't work when the scene doesn't have the focus,
    // e.g. when an assistant is used.
    if (m_commandItem == worksheet()->focusItem()) {
        return evaluate();
    } else if (informationItemHasFocus()) {
        addInformation();
        return true;
    }

    return false;
}

bool CommandEntry::evaluate(EvaluationOption evalOp)
{
    removeContextHelp();
    QToolTip::hideText();

    QString cmd = command();
    m_evaluationOption = evalOp;

    if(cmd.isEmpty()) {
        removeResult();
        foreach(WorksheetTextItem* item, m_informationItems) {
            item->deleteLater();
        }
        m_informationItems.clear();
        recalculateSize();

        evaluateNext(m_evaluationOption);
        return false;
    }

    Cantor::Expression* expr;
    expr = worksheet()->session()->evaluateExpression(cmd);
    connect(expr, SIGNAL(gotResult()), worksheet(), SLOT(gotResult()));

    setExpression(expr);

    return true;
}

void CommandEntry::interruptEvaluation()
{
    Cantor::Expression *expr = expression();
    if(expr)
        expr->interrupt();
}

void CommandEntry::updateEntry()
{
    qDebug() << "update Entry";
    Cantor::Expression* expr = expression();
    if (expr == nullptr || expr->results().isEmpty())
        return;

    if (expr->results().last()->type() == Cantor::HelpResult::Type)
        return; // Help is handled elsewhere

    //CommandEntry::updateEntry() is only called if the worksheet view is resized
    //or when we got a new result.
    //In the second case the number of results and the number of result graphic objects is different
    //and we add a new graphic object for the new result.
    if (expr->results().size() != m_resultItems.size())
    {
        m_resultItems << ResultItem::create(this, expr->results().last());
        animateSizeChange();
    }

    //TODO:old logic, keep it here until the new logic with multiple result objects turns out to be complete.
//     if (expr->result()->type() == Cantor::TextResult::Type &&
//         expr->result()->toHtml().trimmed().isEmpty()) {
//         return;
//     } else if (!m_resultItem) {
//         m_resultItem = ResultItem::create(this, expr->result());
//         qDebug() << "new result";
//         animateSizeChange();
//     } else {
//         m_resultItem = m_resultItem->updateFromResult(expr->result());
//         qDebug() << "update result";
//         animateSizeChange();
//      }
}

void CommandEntry::expressionChangedStatus(Cantor::Expression::Status status)
{
    switch (status)
    {
    case Cantor::Expression::Computing:
    {
        //change the background of the promt item and start animating it (fade in/out).
        //don't start the animation immediately in order to avoid unwanted flickering for "short" commands,
        //start the animation after 1 second passed.
        QTimer::singleShot(1000, this, [=] () {
            KColorScheme scheme = KColorScheme(QPalette::Normal, KColorScheme::View);
            m_promptItem->setBackgroundColor(scheme.decoration(KColorScheme::HoverColor).color());
            m_promtItemTimer->start(100);
        });
        break;
    }
    case Cantor::Expression::Queued:
    {
        QTimer::singleShot(1000, this, [=] () {
            if (m_expression->status() == Cantor::Expression::Queued)
            {
                KColorScheme scheme = KColorScheme(QPalette::Normal, KColorScheme::View);
                m_promptItem->setBackgroundColor(scheme.decoration(KColorScheme::HoverColor).color());
            }
        });
        break;
    }
    case Cantor::Expression::Error:
    case Cantor::Expression::Interrupted:
        m_promtItemTimer->stop();
        m_promptItem->setBackgroundColor(QColor());
        m_promptItem->setOpacity(1.);

        m_commandItem->setFocusAt(WorksheetTextItem::BottomRight, 0);

        if(!m_errorItem)
        {
            m_errorItem = new WorksheetTextItem(this, Qt::TextSelectableByMouse);
        }

        if (status == Cantor::Expression::Error)
            m_errorItem->setHtml(m_expression->errorMessage());
        else
            m_errorItem->setHtml(i18n("Interrupted"));

        recalculateSize();
        break;
    case Cantor::Expression::Done:
        m_promtItemTimer->stop();
        m_promptItem->setBackgroundColor(QColor());
        m_promptItem->setOpacity(1.);
        evaluateNext(m_evaluationOption);
        m_evaluationOption = DoNothing;
        break;
    default:
        break;
    }
}

void CommandEntry::animatePromptItem() {
    if (m_expression->status() != Cantor::Expression::Computing)
    {
        //the expression is not being computed anymore, reset the promt item and leave
        m_promptItem->setBackgroundColor(QColor());
        m_promptItem->setOpacity(1.);
        m_promtItemTimer->stop();
        return;
    }

    static bool fadeOut = true;
    qreal newOpacity = m_promptItem->opacity();
    if (fadeOut)
    {
        newOpacity -= 0.1;
        if (newOpacity < 0.0)
        {
            fadeOut = false;
            return;
        }
    }
    else
    {
        newOpacity += 0.1;
        if (newOpacity > 1.0)
        {
            fadeOut = true;
            return;
        }
    }

    m_promptItem->setOpacity(newOpacity);
}

bool CommandEntry::isEmpty()
{
    if (m_commandItem->toPlainText().trimmed().isEmpty()) {
        if (!m_resultItems.isEmpty())
            return false;
        return true;
    }
    return false;
}

bool CommandEntry::focusEntry(int pos, qreal xCoord)
{
    if (aboutToBeRemoved())
        return false;
    WorksheetTextItem* item;
    if (pos == WorksheetTextItem::TopLeft || pos == WorksheetTextItem::TopCoord)
        item = m_commandItem;
    else if (m_informationItems.size() && currentInformationItem()->isEditable())
        item = currentInformationItem();
    else
        item = m_commandItem;

    item->setFocusAt(pos, xCoord);
    return true;
}

void CommandEntry::setCompletion(Cantor::CompletionObject* tc)
{
    if (m_completionObject)
        delete m_completionObject;

    m_completionObject = tc;
    connect(m_completionObject, &Cantor::CompletionObject::done, this, &CommandEntry::showCompletions);
    connect(m_completionObject, &Cantor::CompletionObject::lineDone, this, &CommandEntry::completeLineTo);
}

void CommandEntry::showCompletions()
{
    disconnect(m_completionObject, &Cantor::CompletionObject::done, this, &CommandEntry::showCompletions);
    QString completion = m_completionObject->completion();
    qDebug()<<"completion: "<<completion;
    qDebug()<<"showing "<<m_completionObject->allMatches();

    if(m_completionObject->hasMultipleMatches())
    {
        completeCommandTo(completion);

        QToolTip::showText(QPoint(), QString(), worksheetView());
        if (!m_completionBox)
               m_completionBox = new KCompletionBox(worksheetView());

        m_completionBox->clear();
        m_completionBox->setItems(m_completionObject->allMatches());
        QList<QListWidgetItem*> items = m_completionBox->findItems(m_completionObject->command(), Qt::MatchFixedString|Qt::MatchCaseSensitive);
        if (!items.empty())
            m_completionBox->setCurrentItem(items.first());
        m_completionBox->setTabHandling(false);
        m_completionBox->setActivateOnSelect(true);
        connect(m_completionBox.data(), &KCompletionBox::activated, this, &CommandEntry::applySelectedCompletion);
        connect(m_commandItem->document(), SIGNAL(contentsChanged()), this, SLOT(completedLineChanged()));
        connect(m_completionObject, &Cantor::CompletionObject::done, this, &CommandEntry::updateCompletions);

        m_commandItem->activateCompletion(true);
        m_completionBox->popup();
        m_completionBox->move(getPopupPosition());
    } else
    {
        completeCommandTo(completion, FinalCompletion);
    }
}

bool CommandEntry::isShowingCompletionPopup()
{
    return m_completionBox && m_completionBox->isVisible();
}

void CommandEntry::applySelectedCompletion()
{
    QListWidgetItem* item = m_completionBox->currentItem();
    if(item)
        completeCommandTo(item->text(), FinalCompletion);
    m_completionBox->hide();
}

void CommandEntry::completedLineChanged()
{
    if (!isShowingCompletionPopup()) {
        // the completion popup is not visible anymore, so let's clean up
        removeContextHelp();
        return;
    }
    const QString line = currentLine();
    m_completionObject->updateLine(line, m_commandItem->textCursor().positionInBlock());
}

void CommandEntry::updateCompletions()
{
    if (!m_completionObject)
        return;
    QString completion = m_completionObject->completion();
    qDebug()<<"completion: "<<completion;
    qDebug()<<"showing "<<m_completionObject->allMatches();

    if(m_completionObject->hasMultipleMatches() || !completion.isEmpty())
    {
        QToolTip::showText(QPoint(), QString(), worksheetView());
        m_completionBox->setItems(m_completionObject->allMatches());
        QList<QListWidgetItem*> items = m_completionBox->findItems(m_completionObject->command(), Qt::MatchFixedString|Qt::MatchCaseSensitive);
        if (!items.empty())
            m_completionBox->setCurrentItem(items.first());

        m_completionBox->move(getPopupPosition());
    } else
    {
        removeContextHelp();
    }
}

void CommandEntry::completeCommandTo(const QString& completion, CompletionMode mode)
{
    qDebug() << "completion: " << completion;

    Cantor::CompletionObject::LineCompletionMode cmode;
    if (mode == FinalCompletion) {
        cmode = Cantor::CompletionObject::FinalCompletion;
        Cantor::SyntaxHelpObject* obj = worksheet()->session()->syntaxHelpFor(completion);
        if(obj)
            setSyntaxHelp(obj);
    } else {
        cmode = Cantor::CompletionObject::PreliminaryCompletion;
        if(m_syntaxHelpObject)
            m_syntaxHelpObject->deleteLater();
        m_syntaxHelpObject=nullptr;
    }

    m_completionObject->completeLine(completion, cmode);
}

void CommandEntry::completeLineTo(const QString& line, int index)
{
    qDebug() << "line completion: " << line;
    QTextCursor cursor = m_commandItem->textCursor();
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    int startPosition = cursor.position();
    cursor.insertText(line);
    cursor.setPosition(startPosition + index);
    m_commandItem->setTextCursor(cursor);

    if (m_syntaxHelpObject) {
        m_syntaxHelpObject->fetchSyntaxHelp();
        // If we are about to show syntax help, then this was the final
        // completion, and we should clean up.
        removeContextHelp();
    }
}

void CommandEntry::setSyntaxHelp(Cantor::SyntaxHelpObject* sh)
{
    if(m_syntaxHelpObject)
        m_syntaxHelpObject->deleteLater();

    m_syntaxHelpObject=sh;
    connect(sh, SIGNAL(done()), this, SLOT(showSyntaxHelp()));
}

void CommandEntry::showSyntaxHelp()
{
    const QString& msg = m_syntaxHelpObject->toHtml();
    const QPointF cursorPos = m_commandItem->cursorPosition();

    QToolTip::showText(toGlobalPosition(cursorPos), msg, worksheetView());
}

void CommandEntry::resultDeleted()
{
    qDebug()<<"result got removed...";
}

void CommandEntry::addInformation()
{
    WorksheetTextItem *answerItem = currentInformationItem();
    answerItem->setTextInteractionFlags(Qt::TextSelectableByMouse);

    QString inf = answerItem->toPlainText();
    inf.replace(QChar::ParagraphSeparator, QLatin1Char('\n'));
    inf.replace(QChar::LineSeparator, QLatin1Char('\n'));

    qDebug()<<"adding information: "<<inf;
    if(m_expression)
        m_expression->addInformation(inf);
}

void CommandEntry::showAdditionalInformationPrompt(const QString& question)
{
    WorksheetTextItem* questionItem = new WorksheetTextItem(this, Qt::TextSelectableByMouse);
    WorksheetTextItem* answerItem = new WorksheetTextItem(this, Qt::TextEditorInteraction);
    questionItem->setPlainText(question);
    m_informationItems.append(questionItem);
    m_informationItems.append(answerItem);
    connect(answerItem, &WorksheetTextItem::moveToPrevious, this, &CommandEntry::moveToPreviousItem);
    connect(answerItem, &WorksheetTextItem::moveToNext, this, &CommandEntry::moveToNextItem);

    connect(answerItem, &WorksheetTextItem::execute, this, &CommandEntry::addInformation);
    answerItem->setFocus();
    recalculateSize();
}

void CommandEntry::removeResult()
{
    //clear the Result objects
    if(m_expression)
    {
        m_expression->clearResult();
    }

    //fade out all result graphic objects
    for(auto* item : m_resultItems)
        fadeOutItem(item->graphicsObject());

    //delete all result graphic objects
    qDeleteAll(m_resultItems);
    m_resultItems.clear();
}

void CommandEntry::removeContextHelp()
{
    disconnect(m_commandItem->document(), SIGNAL(contentsChanged()), this, SLOT(completedLineChanged()));

    m_commandItem->activateCompletion(false);
    if (m_completionBox)
        m_completionBox->hide();
}

void CommandEntry::updatePrompt()
{
    KColorScheme color = KColorScheme( QPalette::Normal, KColorScheme::View);

    m_promptItem->setPlainText(QLatin1String(""));
    QTextCursor c = m_promptItem->textCursor();
    QTextCharFormat cformat = c.charFormat();

    cformat.clearForeground();
    c.setCharFormat(cformat);
    cformat.setFontWeight(QFont::Bold);

    //insert the session id if available
    if(m_expression && worksheet()->showExpressionIds()&&m_expression->id()!=-1)
        c.insertText(QString::number(m_expression->id()),cformat);

    //detect the correct color for the prompt, depending on the
    //Expression state
    if(m_expression)
    {
        if(m_expression ->status() == Cantor::Expression::Computing&&worksheet()->isRunning())
            cformat.setForeground(color.foreground(KColorScheme::PositiveText));
        else if(m_expression ->status() == Cantor::Expression::Error)
            cformat.setForeground(color.foreground(KColorScheme::NegativeText));
        else if(m_expression ->status() == Cantor::Expression::Interrupted)
            cformat.setForeground(color.foreground(KColorScheme::NeutralText));
        else
            cformat.setFontWeight(QFont::Normal);
    }

    c.insertText(CommandEntry::Prompt,cformat);
    recalculateSize();
}

WorksheetTextItem* CommandEntry::currentInformationItem()
{
    if (m_informationItems.isEmpty())
        return nullptr;
    return m_informationItems.last();
}

bool CommandEntry::informationItemHasFocus()
{
    if (m_informationItems.isEmpty())
        return false;
    return m_informationItems.last()->hasFocus();
}

bool CommandEntry::focusWithinThisItem()
{
    return focusItem() != nullptr;
}

QPoint CommandEntry::getPopupPosition()
{
    const QPointF cursorPos = m_commandItem->cursorPosition();
    const QPoint globalPos = toGlobalPosition(cursorPos);
    const QDesktopWidget* desktop = QApplication::desktop();
    const QRect screenRect = desktop->screenGeometry(globalPos);
    if (globalPos.y() + m_completionBox->height() < screenRect.bottom()) {
        return (globalPos);
    } else {
        QTextBlock block = m_commandItem->textCursor().block();
        QTextLayout* layout = block.layout();
        int pos = m_commandItem->textCursor().position() - block.position();
        QTextLine line = layout->lineForTextPosition(pos);
        int dy = - m_completionBox->height() - line.height() - line.leading();
        return QPoint(globalPos.x(), globalPos.y() + dy);
    }
}

void CommandEntry::invalidate()
{
    qDebug() << "ToDo: Invalidate here";
}

bool CommandEntry::wantToEvaluate()
{
    return !isEmpty();
}

QPoint CommandEntry::toGlobalPosition(QPointF localPos)
{
    const QPointF scenePos = mapToScene(localPos);
    const QPoint viewportPos = worksheetView()->mapFromScene(scenePos);
    return worksheetView()->viewport()->mapToGlobal(viewportPos);
}

WorksheetCursor CommandEntry::search(const QString& pattern, unsigned flags,
                                     QTextDocument::FindFlags qt_flags,
                                     const WorksheetCursor& pos)
{
    if (pos.isValid() && pos.entry() != this)
        return WorksheetCursor();

    WorksheetCursor p = pos;
    QTextCursor cursor;
    if (flags & WorksheetEntry::SearchCommand) {
        cursor = m_commandItem->search(pattern, qt_flags, p);
        if (!cursor.isNull())
            return WorksheetCursor(this, m_commandItem, cursor);
    }

    if (p.textItem() == m_commandItem)
        p = WorksheetCursor();

    if (m_errorItem && flags & WorksheetEntry::SearchError) {
        cursor = m_errorItem->search(pattern, qt_flags, p);
        if (!cursor.isNull())
            return WorksheetCursor(this, m_errorItem, cursor);
    }

    if (p.textItem() == m_errorItem)
        p = WorksheetCursor();

    for (auto* resultItem : m_resultItems)
    {
        WorksheetTextItem* textResult = dynamic_cast<WorksheetTextItem*>
            (resultItem);
        if (textResult && flags & WorksheetEntry::SearchResult) {
            cursor = textResult->search(pattern, qt_flags, p);
            if (!cursor.isNull())
                return WorksheetCursor(this, textResult, cursor);
        }
    }

    return WorksheetCursor();
}

void CommandEntry::layOutForWidth(qreal w, bool force)
{
    if (w == size().width() && !force)
        return;

    m_promptItem->setPos(0,0);
    double x = 0 + m_promptItem->width() + HorizontalSpacing;
    double y = 0;
    double width = 0;

    m_commandItem->setGeometry(x,y, w-x);
    width = qMax(width, m_commandItem->width());

    y += qMax(m_commandItem->height(), m_promptItem->height());
    foreach(WorksheetTextItem* information, m_informationItems) {
        y += VerticalSpacing;
        y += information->setGeometry(x,y,w-x);
        width = qMax(width, information->width());
    }

    if (m_errorItem) {
        y += VerticalSpacing;
        y += m_errorItem->setGeometry(x,y,w-x);
        width = qMax(width, m_errorItem->width());
    }

    for (auto* resultItem : m_resultItems)
    {
        if (!resultItem)
            continue;
        y += VerticalSpacing;
        y += resultItem->setGeometry(x, y, w-x);
        width = qMax(width, resultItem->width());
    }
    y += VerticalMargin;

    QSizeF s(x+ width, y);
    if (animationActive()) {
        updateSizeAnimation(s);
    } else {
        setSize(s);
    }
}

void CommandEntry::startRemoving()
{
    m_promptItem->setItemDragable(false);
    WorksheetEntry::startRemoving();
}

WorksheetTextItem* CommandEntry::highlightItem()
{
    return m_commandItem;
}
