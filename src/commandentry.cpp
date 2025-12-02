/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
    SPDX-FileCopyrightText: 2016-2021 Alexander Semke <alexander.semke@web.de>
*/

#include "commandentry.h"
#include "resultitem.h"
#include "loadedexpression.h"
#include "worksheetview.h"
#include "textresultitem.h"
#include "settings.h"
#include "lib/backend.h"
#include "lib/jupyterutils.h"
#include "lib/result.h"
#include "lib/helpresult.h"
#include "lib/epsresult.h"
#include "lib/latexresult.h"
#include "lib/syntaxhelpobject.h"
#include "lib/session.h"
#include "worksheettextitem.h"
#include "worksheettexteditoritem.h"

#include <QGuiApplication>
#include <QDebug>
#include <QActionGroup>
#include <QFontDatabase>
#include <QFontDialog>
#include <QScreen>
#include <QTimer>
#include <QToolTip>
#include <QPropertyAnimation>
#include <QJsonArray>
#include <QJsonObject>
#include <QTextBlock>
#include <QTextDocumentFragment>
#include <QPainter>

#include <KLocalizedString>
#include <KColorScheme>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Definition>

const QString CommandEntry::Prompt     = QLatin1String(">>> ");
const QString CommandEntry::MidPrompt  = QLatin1String(">>  ");
const QString CommandEntry::HidePrompt = QLatin1String(">   ");
const double CommandEntry::VerticalSpacing = 4;

namespace {
    QString getThemeNameFromIndex (int index)
    {
        if (index <= 0)
            return QString ();
        const auto& repository = KTextEditor::Editor::instance()->repository();
        const auto& themes = repository.themes();

        const int themeListIndex = index - 1;
        if (themeListIndex>= 0 && themeListIndex < themes.count ())
            return themes.at (themeListIndex).name ();

        return QString ();
    }
}

CommandEntry::CommandEntry(Worksheet* worksheet) : WorksheetEntry(worksheet),
    m_promptItem(new WorksheetTextItem(this, Qt::NoTextInteraction)),
    m_commandItem(new WorksheetTextEditorItem(WorksheetTextEditorItem::Editable, this, this)),
    m_resultsCollapsed(false),
    m_errorItem(nullptr),
    m_expression(nullptr),
    m_evaluationOption(DoNothing),
    m_menusInitialized(false),
    m_textColorCustom(false),
    m_backgroundColorCustom(false),
    m_backgroundColorActionGroup(nullptr),
    m_backgroundColorMenu(nullptr),
    m_textColorActionGroup(nullptr),
    m_textColorMenu(nullptr),
    m_fontMenu(nullptr),
    m_isExecutionEnabled(true),
    m_dynamicHighlighter(nullptr)
{
    m_promptItem->setPlainText(Prompt);
    m_promptItem->setItemDragable(true);;
    m_commandItem->enableCompletion(true);

    if(worksheet && worksheet->session() && worksheet->session()->backend())
    {
        QString backendName = worksheet->session()->backend()->name();
        QString highlightingMode = backendName;

        if (backendName == QLatin1String("R"))
            highlightingMode = QLatin1String("R Script");
        else if(backendName == QLatin1String("Sage"))
            highlightingMode = QLatin1String("Python");

        m_commandItem->setSyntaxHighlightingMode(highlightingMode);

        if (worksheet)
        {
            const auto& parentTheme = worksheet->theme();
            m_commandItem->setTheme(parentTheme.name());
        }

        if (worksheet && worksheet->session() && worksheet->session()->variableModel())
        {
            const auto* ws = this->worksheet();
            const auto& theme = ws->theme();
            const QColor variableColor = theme.textColor(KSyntaxHighlighting::Theme::DataType);
            const QColor functionColor = theme.textColor(KSyntaxHighlighting::Theme::Function);
            m_dynamicHighlighter = new DynamicHighlighter(m_commandItem->document(), ws->session()->variableModel(), variableColor, functionColor, this);
            connect(m_commandItem->document(), &KTextEditor::Document::textChanged, m_dynamicHighlighter, &DynamicHighlighter::updateAllHighlights);
            m_dynamicHighlighter->updateAllHighlights();
        }
    }

    m_promptItemAnimation = new QPropertyAnimation(m_promptItem, "opacity", this);
    m_promptItemAnimation->setDuration(600);
    m_promptItemAnimation->setStartValue(1);
    m_promptItemAnimation->setKeyValueAt(0.5, 0);
    m_promptItemAnimation->setEndValue(1);
    connect(m_promptItemAnimation, &QPropertyAnimation::finished, this, &CommandEntry::animatePromptItem);

    m_promptItem->setDoubleClickBehaviour(WorksheetTextItem::Simple);
    connect(m_promptItem, &WorksheetTextItem::doubleClick, this, &CommandEntry::changeResultCollapsingAction);

    connect(&m_controlElement, &WorksheetControlItem::doubleClick, this, &CommandEntry::changeResultCollapsingAction);
    connect(m_commandItem, &WorksheetTextEditorItem::execute, this, [=]() { evaluate();} );
    connect(m_commandItem, &WorksheetTextEditorItem::moveToPrevious, this, &CommandEntry::moveToPreviousItem);
    connect(m_commandItem, &WorksheetTextEditorItem::moveToNext, this, &CommandEntry::moveToNextItem);
    connect(m_promptItem, &WorksheetTextItem::drag, this, &CommandEntry::startDrag);
    connect(worksheet, &Worksheet::updatePrompt, this, [=]() { updatePrompt();} );

    m_defaultDefaultTextColor = m_commandItem->defaultTextColor();
    updatePrompt();
}

CommandEntry::~CommandEntry()
{
    if (m_menusInitialized)
    {
        m_backgroundColorMenu->deleteLater();
        m_textColorMenu->deleteLater();
        m_fontMenu->deleteLater();
    }
}

int CommandEntry::type() const
{
    return Type;
}

void CommandEntry::initMenus()
{
    m_backgroundColorActionGroup = new QActionGroup(this);
    m_backgroundColorActionGroup->setExclusive(true);
    connect(m_backgroundColorActionGroup, &QActionGroup::triggered, this, &CommandEntry::backgroundColorChanged);

    m_backgroundColorMenu = new QMenu(i18n("Background Color"));
    m_backgroundColorMenu->setIcon(QIcon::fromTheme(QLatin1String("format-fill-color")));

    {
        QPixmap pix(16, 16);
        QPainter p(&pix);
        KColorScheme scheme = KColorScheme(QPalette::Normal, KColorScheme::View);
        p.fillRect(pix.rect(), scheme.background(KColorScheme::AlternateBackground).color());
        QAction* action = new QAction(QIcon(pix), i18n("Default"), m_backgroundColorActionGroup);
        action->setCheckable(true);
        m_backgroundColorMenu->addAction(action);
        if (!m_backgroundColorCustom)
            action->setChecked(true);
    }

    for (int i = 0; i < colorsCount; ++i)
    {
        QPixmap pix(16, 16);
        QPainter p(&pix);
        p.fillRect(pix.rect(), colors[i]);
        QAction* action = new QAction(QIcon(pix), colorNames[i], m_backgroundColorActionGroup);
        action->setCheckable(true);
        m_backgroundColorMenu->addAction(action);
        const QColor& backgroundColor = (m_isExecutionEnabled ? m_commandItem->backgroundColor() : m_activeExecutionBackgroundColor);
        if (m_backgroundColorCustom && backgroundColor == colors[i])
            action->setChecked(true);
    }

    m_textColorActionGroup = new QActionGroup(this);
    m_textColorActionGroup->setExclusive(true);
    connect(m_textColorActionGroup, &QActionGroup::triggered, this, &CommandEntry::textColorChanged);

    m_textColorMenu = new QMenu(i18n("Text Color"));
    m_textColorMenu->setIcon(QIcon::fromTheme(QLatin1String("format-text-color")));

    {
        QPixmap pix(16, 16);
        QPainter p(&pix);
        p.fillRect(pix.rect(), m_commandItem->themeDefaultTextColor());
        QAction* action = new QAction(QIcon(pix), i18n("Default"), m_textColorActionGroup);
        action->setCheckable(true);
        m_textColorMenu->addAction(action);
        if (!m_textColorCustom)
            action->setChecked(true);
    }

    for (int i = 0; i < colorsCount; ++i)
    {
        QPixmap pix(16, 16);
        QPainter p(&pix);
        p.fillRect(pix.rect(), colors[i]);
        QAction* action = new QAction(QIcon(pix), colorNames[i], m_textColorActionGroup);
        action->setCheckable(true);
        m_textColorMenu->addAction(action);
        const QColor& textColor = (m_isExecutionEnabled ? m_commandItem->defaultTextColor() : m_activeExecutionTextColor);
        if (m_textColorCustom && textColor == colors[i])
            action->setChecked(true);
    }

    QFont font = m_commandItem->font();
    m_fontMenu = new QMenu(i18n("Font"));
    m_fontMenu->setIcon(QIcon::fromTheme(QLatin1String("preferences-desktop-font")));

    QAction* action = new QAction(QIcon::fromTheme(QLatin1String("format-text-bold")), i18n("Bold"));
    action->setCheckable(true);
    connect(action, &QAction::triggered, this, &CommandEntry::fontBoldTriggered);
    m_fontMenu->addAction(action);
    if (font.bold())
        action->setChecked(true);

    action = new QAction(QIcon::fromTheme(QLatin1String("format-text-italic")), i18n("Italic"));
    action->setCheckable(true);
    connect(action, &QAction::triggered, this, &CommandEntry::fontItalicTriggered);
    m_fontMenu->addAction(action);
    if (font.italic())
        action->setChecked(true);

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

    action = new QAction(QIcon::fromTheme(QLatin1String("preferences-desktop-font")), i18n("Reset to Default"));
    connect(action, &QAction::triggered, this, &CommandEntry::resetFontTriggered);
    m_fontMenu->addAction(action);

    m_menusInitialized = true;
}

void CommandEntry::backgroundColorChanged(QAction* action) {
    int index = m_backgroundColorActionGroup->actions().indexOf(action);

    if (index == 0)
    {
        m_commandItem->setBackgroundColor(QColor());
        m_backgroundColorCustom = false;
    }
    else
    {
        const QColor color = colors[index - 1];
        m_commandItem->setBackgroundColor(color);
        m_backgroundColorCustom = true;
    }
}

void CommandEntry::textColorChanged(QAction* action)
{
    int index = m_textColorActionGroup->actions().indexOf(action);

    if (index == 0)
    {
        m_commandItem->setDefaultTextColor(QColor());
    }
    else
    {
        const QColor color = colors[index - 1];
        m_commandItem->setDefaultTextColor(color);
    }
}

void CommandEntry::fontBoldTriggered()
{
    QAction* action = static_cast<QAction*>(QObject::sender());
    m_commandItem->setTextBold(action->isChecked());
}

void CommandEntry::resetFontTriggered()
{
    m_commandItem->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
}

void CommandEntry::fontItalicTriggered()
{
    QAction* action = static_cast<QAction*>(QObject::sender());
    m_commandItem->setTextItalic(action->isChecked());
}

void CommandEntry::fontIncreaseTriggered()
{
    m_commandItem->increaseFontSize();
}

void CommandEntry::fontDecreaseTriggered()
{
    m_commandItem->decreaseFontSize();
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

    if (!m_resultItems.isEmpty()) {
        if (m_resultsCollapsed)
            menu->addAction(i18n("Show Results"), this, &CommandEntry::expandResults);
        else
            menu->addAction(i18n("Hide Results"), this, &CommandEntry::collapseResults);
    }

    if (!m_commandItem->toPlainText().simplified().isEmpty())
    {
        QAction* action = new QAction(QIcon::fromTheme(QLatin1String("help-hint")), i18n("Show Help"));
        connect(action, &QAction::triggered, this, &CommandEntry::showHelp);
        menu->addAction(action);
        menu->addSeparator();
    }

    QAction* enabledAction = new QAction(QIcon::fromTheme(QLatin1String("checkmark")), i18n("Enabled"));
    enabledAction->setCheckable(true);
    enabledAction->setChecked(m_isExecutionEnabled);
    menu->addSeparator();
    menu->addAction(enabledAction);
    connect(enabledAction, &QAction::triggered, this, &CommandEntry::toggleEnabled);

    QMenu* appearanceMenu = new QMenu(i18n("Appearance"));
    appearanceMenu->setIcon(QIcon::fromTheme(QLatin1String("configure")));
    appearanceMenu->addMenu(m_backgroundColorMenu);
    appearanceMenu->addMenu(m_textColorMenu);
    appearanceMenu->addMenu(m_fontMenu);
    menu->addMenu(appearanceMenu);
    menu->addSeparator();

    WorksheetEntry::populateMenu(menu, pos);
    menu->addSeparator();
}

void CommandEntry::updateAfterSettingsChanges()
{
    WorksheetEntry::updateAfterSettingsChanges();
    const auto& parentTheme = worksheet()->theme();
    const QString themeName = parentTheme.name();
    m_commandItem->setTheme(themeName);

    updatePrompt();

    if (m_dynamicHighlighter)
    {
        const auto* ws = this->worksheet();
        const auto& theme = ws->theme();
        const QColor variableColor = theme.textColor(KSyntaxHighlighting::Theme::DataType);
        const QColor functionColor = theme.textColor(KSyntaxHighlighting::Theme::Function);
        m_dynamicHighlighter->updateThemeColors(variableColor, functionColor);
    }

    for (auto* item : m_resultItems)
    {
        if (item)
            item->updateTheme();
    }
}

void CommandEntry::moveToNextItem(int pos, qreal x)
{
    auto* item = qobject_cast<WorksheetTextEditorItem*>(sender());
    if (!item)
        return;

    if (item == m_commandItem)
    {
        if (m_informationItems.isEmpty() ||
            !currentInformationItem()->isEditable())
            moveToNextEntry(pos, x);
        else
            currentInformationItem()->setFocusAt(pos, x);
    }
    else {
        auto* infoItem = qobject_cast<WorksheetTextItem*>(sender());
        if (infoItem && infoItem == currentInformationItem())
            moveToNextEntry(pos, x);
    }
}

void CommandEntry::moveToPreviousItem(int pos, qreal x)
{
    auto* editorItem = qobject_cast<WorksheetTextEditorItem*>(sender());
    if (editorItem && editorItem == m_commandItem)
    {
        moveToPreviousEntry(pos, x);
        return;
    }

    auto* textItem = qobject_cast<WorksheetTextItem*>(sender());
    if (textItem && textItem == currentInformationItem())
        m_commandItem->setFocusAt(pos, x);
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


    for (auto* item : m_informationItems)
    {
        item->deleteLater();
    }
    m_informationItems.clear();

    // Delete any previous result
    clearResultItems();

    m_expression = expr;
    m_resultsCollapsed = false;

    connect(expr, &Cantor::Expression::gotResult, this, &CommandEntry::updateEntry);
    connect(expr, &Cantor::Expression::resultsCleared, this, &CommandEntry::clearResultItems);
    connect(expr, &Cantor::Expression::resultRemoved, this, &CommandEntry::removeResultItem);
    connect(expr, &Cantor::Expression::resultReplaced, this, &CommandEntry::replaceResultItem);
    connect(expr, &Cantor::Expression::idChanged, this,  [=]() { updatePrompt();} );
    connect(expr, &Cantor::Expression::statusChanged, this, &CommandEntry::expressionChangedStatus);
    connect(expr, &Cantor::Expression::needsAdditionalInformation, this, &CommandEntry::showAdditionalInformationPrompt);
    connect(expr, &Cantor::Expression::statusChanged, this,  [=]() { updatePrompt();} );

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

    LoadedExpression* expr = new LoadedExpression( worksheet()->session() );
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
        m_backgroundColorCustom = true;
    }

    //text properties
    QDomElement textElem = content.firstChildElement(QLatin1String("Text"));
    if (!textElem.isNull())
    {
        //text color
        QDomElement colorElem = textElem.firstChildElement(QLatin1String("Color"));
        if (!colorElem.isNull() && !colorElem.hasAttribute(QLatin1String("default")))
        {
            m_defaultDefaultTextColor = m_commandItem->defaultTextColor();
            QColor color;
            color.setRed(colorElem.attribute(QLatin1String("red")).toInt());
            color.setGreen(colorElem.attribute(QLatin1String("green")).toInt());
            color.setBlue(colorElem.attribute(QLatin1String("blue")).toInt());
            m_commandItem->setDefaultTextColor(color);
            m_textColorCustom = true;
        }

        //font properties
        QDomElement fontElem = textElem.firstChildElement(QLatin1String("Font"));
        if (!fontElem.isNull() && !fontElem.hasAttribute(QLatin1String("default")))
        {
            QFont font;
            font.setFamily(fontElem.attribute(QLatin1String("family")));
            font.setPointSize(fontElem.attribute(QLatin1String("pointSize")).toInt());
            font.setWeight(static_cast<QFont::Weight>(fontElem.attribute(QLatin1String("weight")).toInt()));
            font.setItalic(fontElem.attribute(QLatin1String("italic")).toInt());
            m_commandItem->setFont(font);
        }
    }

    m_isExecutionEnabled = !(bool)(content.attribute(QLatin1String("ExecutionDisabled"), QLatin1String("0")).toInt());
    if (m_isExecutionEnabled == false)
        excludeFromExecution();

    setExpression(expr);

}

void CommandEntry::setContentFromJupyter(const QJsonObject& cell)
{
    m_commandItem->setPlainText(Cantor::JupyterUtils::getSource(cell));

    LoadedExpression* expr=new LoadedExpression( worksheet()->session() );
    expr->loadFromJupyter(cell);
    setExpression(expr);

    // https://nbformat.readthedocs.io/en/latest/format_description.html#cell-metadata
    // 'collapsed': +
    // 'scrolled', 'deletable', 'name', 'tags' don't supported Cantor, so ignore them
    // 'source_hidden' don't supported
    // 'format' for raw entry, so ignore
    // I haven't found 'outputs_hidden' inside Jupyter notebooks, and difference from 'collapsed'
    // not clear, so also ignore
    const QJsonObject& metadata = Cantor::JupyterUtils::getMetadata(cell);
    const QJsonValue& collapsed = metadata.value(QLatin1String("collapsed"));
    if (collapsed.isBool() && collapsed.toBool() == true && !m_resultItems.isEmpty())
    {
        // Disable animation for hiding results, we don't need animation on document load stage
        bool animationValue = worksheet()->animationsEnabled();
        worksheet()->enableAnimations(false);
        collapseResults();
        worksheet()->enableAnimations(animationValue);
    }

    setJupyterMetadata(metadata);
}

QJsonValue CommandEntry::toJupyterJson()
{
    QJsonObject entry;

    entry.insert(QLatin1String("cell_type"), QLatin1String("code"));

    QJsonValue executionCountValue;
    if (expression() && expression()->id() != -1)
        executionCountValue = QJsonValue(expression()->id());
    entry.insert(QLatin1String("execution_count"), executionCountValue);

    QJsonObject metadata(jupyterMetadata());
    if (m_resultsCollapsed)
        metadata.insert(QLatin1String("collapsed"), true);

    entry.insert(QLatin1String("metadata"), metadata);

    Cantor::JupyterUtils::setSource(entry, command());

    QJsonArray outputs;
    if (expression())
    {
        Cantor::Expression::Status status = expression()->status();
        if (status == Cantor::Expression::Error || status == Cantor::Expression::Interrupted)
        {
            QJsonObject errorOutput;
            errorOutput.insert(Cantor::JupyterUtils::outputTypeKey, QLatin1String("error"));
            errorOutput.insert(QLatin1String("ename"), QLatin1String("Unknown"));
            errorOutput.insert(QLatin1String("evalue"), QLatin1String("Unknown"));

            QJsonArray traceback;
            if (status == Cantor::Expression::Error)
            {
                const QStringList& error = expression()->errorMessage().split(QLatin1Char('\n'));
                for (const QString& line: error)
                    traceback.append(line);
            }
            else
                traceback.append(i18n("Interrupted"));
            errorOutput.insert(QLatin1String("traceback"), traceback);

            outputs.append(errorOutput);
        }

        for (auto* result : expression()->results())
        {
            const QJsonValue& resultJson = result->toJupyterJson();

            if (!resultJson.isNull())
                outputs.append(resultJson);
        }
    }
    entry.insert(QLatin1String("outputs"), outputs);

    return entry;
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

    if (!m_isExecutionEnabled)
        exprElem.setAttribute(QLatin1String("ExecutionDisabled"), true);

    // save results of the expression if they exist
    if (expression())
    {
        const QString& errorMessage = expression()->errorMessage();
        if (!errorMessage.isEmpty())
        {
            QDomElement errorElem = doc.createElement( QLatin1String("Error") );
            errorElem.appendChild(doc.createTextNode(errorMessage));
            exprElem.appendChild(errorElem);
        }
        for (auto* result : expression()->results())
        {
            const QDomElement& resultElem = result->toXml(doc);
            exprElem.appendChild(resultElem);

            if (archive)
                result->saveAdditionalData(archive);
        }
    }

    bool isBackgroundColorNotDefault = false;
    // If user can change value from menu (menus have been inited) - check via menu
    // If use don't have menu, check if loaded color was custom color
    if (m_backgroundColorActionGroup)
        isBackgroundColorNotDefault = m_backgroundColorActionGroup->actions().indexOf(m_backgroundColorActionGroup->checkedAction()) != 0;
    else
        isBackgroundColorNotDefault = m_backgroundColorCustom;
    if (isBackgroundColorNotDefault)
    {
        QColor backgroundColor = (m_isExecutionEnabled ? m_commandItem->backgroundColor() : m_activeExecutionBackgroundColor);
        QDomElement colorElem = doc.createElement( QLatin1String("Background") );
        colorElem.setAttribute(QLatin1String("red"), QString::number(backgroundColor.red()));
        colorElem.setAttribute(QLatin1String("green"), QString::number(backgroundColor.green()));
        colorElem.setAttribute(QLatin1String("blue"), QString::number(backgroundColor.blue()));
        exprElem.appendChild(colorElem);
    }

    //save the text properties if they differ from default values
    const QFont& font = m_commandItem->font();
    const QColor& textColor = (m_isExecutionEnabled ? m_commandItem->defaultTextColor() : m_activeExecutionTextColor);
    bool isFontNotDefault = font != QFontDatabase::systemFont(QFontDatabase::FixedFont);

    bool isTextColorNotDefault = false;
    if (m_textColorActionGroup)
        isTextColorNotDefault = m_textColorActionGroup->actions().indexOf(m_textColorActionGroup->checkedAction()) != 0;
    else
        isTextColorNotDefault = m_textColorCustom;

    // Setting both values is necessary for previous Cantor versions compatibility
    // Value, added only for compatibility reason, marks with attribute
    if (isFontNotDefault || isTextColorNotDefault)
    {
        QDomElement textElem = doc.createElement(QLatin1String("Text"));

        //font properties
        QDomElement fontElem = doc.createElement(QLatin1String("Font"));
        if (!isFontNotDefault)
            fontElem.setAttribute(QLatin1String("default"), true);
        fontElem.setAttribute(QLatin1String("family"), font.family());
        fontElem.setAttribute(QLatin1String("pointSize"), QString::number(font.pointSize()));
        fontElem.setAttribute(QLatin1String("weight"), QString::number(font.weight()));
        fontElem.setAttribute(QLatin1String("italic"), QString::number(font.italic()));
        textElem.appendChild(fontElem);

        //text color
        QDomElement colorElem = doc.createElement( QLatin1String("Color") );
        if (!isTextColorNotDefault)
            colorElem.setAttribute(QLatin1String("default"), true);
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

    KTextEditor::View *view = m_commandItem->view();
    KTextEditor::Cursor cursor = view->cursorPosition();

    return view->document()->line(cursor.line());
}

bool CommandEntry::evaluateCurrentItem()
{
    // we can't use m_commandItem->hasFocus() here, because
    // that doesn't work when the scene doesn't have the focus,
    // e.g. when an assistant is used.
    if (m_commandItem == worksheet()->focusItem())
        return evaluate();
    else if (informationItemHasFocus())
    {
        addInformation();
        return true;
    }

    return false;
}

bool CommandEntry::evaluate(EvaluationOption evalOp)
{
    if (m_isExecutionEnabled)
    {
        if (worksheet()->session()->status() == Cantor::Session::Disable)
            worksheet()->loginToSession();

        QToolTip::hideText();

        QString cmd = command();
        m_evaluationOption = evalOp;

        if(cmd.isEmpty()) {
            removeResults();
            for (auto* item : m_informationItems) 
                item->deleteLater();
            m_informationItems.clear();
            recalculateSize();

            evaluateNext(m_evaluationOption);
            return false;
        }

        auto* expr = worksheet()->session()->evaluateExpression(cmd);
        connect(expr, &Cantor::Expression::gotResult, this, [=]() { worksheet()->gotResult(expr); });

        setExpression(expr);

        return true;
    }
    else
    {
        evaluateNext(m_evaluationOption);
        return true;
    }
}

void CommandEntry::interruptEvaluation()
{
    auto* expr = expression();
    if(expr)
        expr->interrupt();
}

void CommandEntry::updateEntry()
{
    qDebug() << "update Entry";
    auto* expr = expression();
    if (expr == nullptr || expr->results().isEmpty())
        return;

    if (expr->results().last()->type() == Cantor::HelpResult::Type)
        return; // Help is handled elsewhere

    //CommandEntry::updateEntry() is only called if the worksheet view is resized
    //or when we got a new result(s).
    //In the second case the number of results and the number of result graphic objects is different
    //and we add a new graphic objects for the new result(s) (new result(s) are located in the end).
    // NOTE: LatexResult could request update (change from rendered to code, for example)
    // So, just update results, if we haven't new results or something similar
    if (m_resultItems.size() < expr->results().size())
    {
        if (m_resultsCollapsed)
            expandResults();

        for (int i = m_resultItems.size(); i < expr->results().size(); i++)
            m_resultItems << ResultItem::create(this, expr->results()[i]);
    }
    else
    {
        for (ResultItem* item: m_resultItems)
            item->update();
    }

    m_controlElement.isCollapsable = m_errorItem != nullptr
                                    || m_informationItems.size() > 0;
    animateSizeChange();
}

void CommandEntry::expressionChangedStatus(Cantor::Expression::Status status)
{
    switch (status)
    {
        case Cantor::Expression::Computing:
        {
            //change the background of the prompt item and start animating it (fade in/out).
            //don't start the animation immediately in order to avoid unwanted flickering for "short" commands,
            //start the animation after 1 second passed.
            if (worksheet()->animationsEnabled())
            {
                const int id = m_expression->id();
                QTimer::singleShot(1000, this, [this, id] () {
                    if(m_expression->status() == Cantor::Expression::Computing && m_expression->id() == id)
                        m_promptItemAnimation->start();
                });
            }
            break;
        }
        case Cantor::Expression::Error:
        case Cantor::Expression::Interrupted:
            m_promptItemAnimation->stop();
            m_promptItem->setOpacity(1.);
            m_commandItem->setFocusAt(WorksheetTextEditorItem::BottomRight, 0);

            if(!m_errorItem)
                m_errorItem = new WorksheetTextItem(this, Qt::TextSelectableByMouse);

            if (status == Cantor::Expression::Error)
            {
                QString error = m_expression->errorMessage().toHtmlEscaped();
                while (error.endsWith(QLatin1Char('\n')))
                    error.chop(1);
                error.replace(QLatin1String("\n"), QLatin1String("<br>"));
                error.replace(QLatin1String(" "), QLatin1String("&nbsp;"));
                m_errorItem->setHtml(error);
            }
            else
                m_errorItem->setHtml(i18n("Interrupted"));

        recalculateSize();
        // Mostly we handle setting of modification in WorksheetEntry inside ::evaluateNext.
        // But command entry wouldn't trigger ::evaluateNext for Error and Interrupted states
        // So, we set it here
        worksheet()->setModified();
        break;
        case Cantor::Expression::Done:
            m_promptItemAnimation->stop();
            m_promptItem->setOpacity(1.);
            evaluateNext(m_evaluationOption);
            m_evaluationOption = DoNothing;
            break;
        default:
            break;
    }
}


void CommandEntry::animatePromptItem() {
    if(m_expression->status() == Cantor::Expression::Computing)
        m_promptItemAnimation->start();
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
    if (pos == WorksheetTextItem::TopLeft || pos == WorksheetTextItem::TopCoord)
        m_commandItem->setFocusAt(pos, xCoord);
    else if (m_informationItems.size() && currentInformationItem()->isEditable())
        currentInformationItem()->setFocusAt(pos, xCoord);
    else
        m_commandItem->setFocusAt(pos, xCoord);

    return true;
}

void CommandEntry::resultDeleted()
{
    qDebug()<<"result got removed...";
}

void CommandEntry::addInformation()
{
    auto* answerItem = currentInformationItem();
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
    auto* questionItem = new WorksheetTextItem(this, Qt::TextSelectableByMouse);
    auto* answerItem = new WorksheetTextItem(this, Qt::TextEditorInteraction);

    //change the color and the font for when asking for additional information in order to
    //better discriminate from the usual input in the command entry
    KColorScheme scheme = KColorScheme(QPalette::Normal, KColorScheme::View);
    QColor color = scheme.foreground(KColorScheme::PositiveText).color();

    QFont font;
    font.setItalic(true);

    questionItem->setFont(font);
    questionItem->setDefaultTextColor(color);
    answerItem->setFont(font);
    answerItem->setDefaultTextColor(color);

    questionItem->setPlainText(question);

    m_informationItems.append(questionItem);
    m_informationItems.append(answerItem);
    connect(answerItem, &WorksheetTextItem::moveToPrevious, this, &CommandEntry::moveToPreviousItem);
    connect(answerItem, &WorksheetTextItem::moveToNext, this, &CommandEntry::moveToNextItem);
    connect(answerItem, &WorksheetTextItem::execute, this, &CommandEntry::addInformation);
    answerItem->setFocus();

    recalculateSize();

}

void CommandEntry::removeResults()
{
    //clear the Result objects
    if(m_expression)
        m_expression->clearResults();
}

void CommandEntry::removeResult(Cantor::Result* result)
{
    if (m_expression)
        m_expression->removeResult(result);
}

void CommandEntry::removeResultItem(int index)
{
    fadeOutItem(m_resultItems[index]->graphicsObject());
    m_resultItems.remove(index);
    recalculateSize();
}

void CommandEntry::clearResultItems()
{
    //fade out all result graphic objects
    for(auto* item : m_resultItems)
        fadeOutItem(item->graphicsObject());

    m_resultItems.clear();
    recalculateSize();
}

void CommandEntry::replaceResultItem(int index)
{
    auto* previousItem = m_resultItems[index];
    m_resultItems[index] = ResultItem::create(this, m_expression->results()[index]);
    previousItem->deleteLater();
    recalculateSize();
}

void CommandEntry::updatePrompt(const QString& postfix)
{
    m_promptItem->setPlainText(QString());
    QTextCursor c = m_promptItem->textCursor();
    QTextCharFormat cformat = c.charFormat();

    const auto* ws = worksheet();
    if (ws) {
        const auto& theme = ws->theme();

        QColor promptColor = theme.textColor(KSyntaxHighlighting::Theme::TextStyle::Comment);

        if (!promptColor.isValid())
            promptColor = theme.editorColor(KSyntaxHighlighting::Theme::EditorColorRole::LineNumbers);

        if (!promptColor.isValid())
            promptColor = theme.textColor(KSyntaxHighlighting::Theme::TextStyle::Normal);

        if (promptColor.isValid())
            cformat.setForeground(promptColor);
    }

    cformat.setFontWeight(QFont::Bold);

    if(m_expression && worksheet()->showExpressionIds() && m_expression->id() != -1)
        c.insertText(QString::number(m_expression->id()), cformat);

    if(m_expression)
    {
        KColorScheme kcolor(QPalette::Normal, KColorScheme::View);

        if(m_expression->status() == Cantor::Expression::Computing && worksheet()->isRunning())
            cformat.setForeground(kcolor.foreground(KColorScheme::PositiveText));
        else if(m_expression->status() == Cantor::Expression::Queued)
            cformat.setForeground(kcolor.foreground(KColorScheme::InactiveText));
        else if(m_expression->status() == Cantor::Expression::Error)
            cformat.setForeground(kcolor.foreground(KColorScheme::NegativeText));
        else if(m_expression->status() == Cantor::Expression::Interrupted)
            cformat.setForeground(kcolor.foreground(KColorScheme::NeutralText));
    }

    c.insertText(postfix, cformat);

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
    if (!(flags & SearchResult))
        return WorksheetCursor();

    int startIndex = 0;
    bool searchFromMiddle = false;

    if (pos.isValid() && pos.entry() == this) {
        for (int i = 0; i < m_resultItems.size(); ++i) {
            if (auto* textResult = dynamic_cast<TextResultItem*>(m_resultItems[i]))
            {
                if (textResult == pos.textItem())
                {
                    startIndex = i;
                    searchFromMiddle = true;
                    break;
                }
            }
        }
    }

    for (int i = startIndex; i < m_resultItems.size(); ++i)
    {
        if (auto* textResult = dynamic_cast<TextResultItem*>(m_resultItems[i]))
        {
            WorksheetCursor startPosInResult = (searchFromMiddle && i == startIndex) ? pos : WorksheetCursor();

            QTextCursor found = textResult->search(pattern, qt_flags, startPosInResult);
            if (!found.isNull())
                return WorksheetCursor(this, textResult, found);
        }
    }

    return WorksheetCursor();
}

KWorksheetCursor CommandEntry::search(const QString& pattern, unsigned flags,
                                      KTextEditor::SearchOptions options,
                                      const KWorksheetCursor& pos)
{
    if (!(flags & SearchCommand) || (pos.isValid() && pos.entry() != this)) 
        return KWorksheetCursor();

    KTextEditor::Cursor startCursor;
    if (pos.isValid() && pos.textItem() == m_commandItem) 
        startCursor = pos.cursor();
    else if (options & KTextEditor::Backwards) 
        startCursor = m_commandItem->document()->documentEnd();

    KTextEditor::Range foundRange = m_commandItem->search(pattern, options, startCursor);

    if (foundRange.isValid()) 
        return KWorksheetCursor(this, m_commandItem, foundRange.start(), foundRange);

    return KWorksheetCursor();
}

bool CommandEntry::replace(const QString& replacement)
{
    if (m_commandItem->isEditable() && m_commandItem->view()->selection())
        return m_commandItem->replace(replacement);
    return false;
}

void CommandEntry::replaceAll(const QString& pattern, const QString& replacement, unsigned flags, KTextEditor::SearchOptions options)
{
    if ((flags & SearchCommand) && m_commandItem->isEditable())
        m_commandItem->replaceAll(pattern, replacement, options);
}

void CommandEntry::layOutForWidth(qreal entry_zone_x, qreal w, bool force)
{
    if (w == size().width() && m_commandItem->pos().x() == entry_zone_x && !force)
        return;

    double x = std::max(0 + m_promptItem->width() + HorizontalSpacing, entry_zone_x);
    double y = 0;
    double width = 0;

    const qreal margin = worksheet()->isPrinting() ? 0 : RightMargin;

    m_commandItem->setGeometry(x, 0, w - x - margin);
    width = qMax(width, m_commandItem->width() + margin);

    const QFont font = m_commandItem->view()->font();
    const qreal singleLineHeight = QFontMetrics(font).height();
    const qreal promptItemHeight = m_promptItem->height();

    qreal promptY = 0;
    qreal editorY = 0;

    if (singleLineHeight > promptItemHeight) 
        promptY = (singleLineHeight - promptItemHeight) / 2.0;
    else 
        editorY = (promptItemHeight - singleLineHeight) / 2.0;

    m_promptItem->setPos(0, promptY);
    m_commandItem->setPos(x, editorY);

    y = std::max(promptY + promptItemHeight, editorY + m_commandItem->height());

    for (auto* item : m_informationItems)
    {
        y += VerticalSpacing;
        y += item->setGeometry(x, y, w - x - margin);
        width = qMax(width, item->width() + margin);
    }

    if (m_errorItem)
    {
        y += VerticalSpacing;
        y += m_errorItem->setGeometry(x,y,w - x - margin);
        width = qMax(width, m_errorItem->width() + margin);
    }


    for (auto* resultItem : m_resultItems)
    {
        if (!resultItem || !resultItem->graphicsObject()->isVisible())
            continue;
        y += VerticalSpacing;
        y += resultItem->setGeometry(x, y, w - x - margin);
        width = qMax(width, resultItem->width() + margin);
    }
    y += VerticalMargin;

    QSizeF s(x+ width, y);
    if (animationActive())
        updateSizeAnimation(s);
    else
        setSize(s);
}

void CommandEntry::startRemoving(bool warn)
{
    m_promptItem->setItemDragable(false);
    WorksheetEntry::startRemoving(warn);
}

WorksheetTextEditorItem* CommandEntry::highlightItem()
{
    return m_isExecutionEnabled ? m_commandItem : nullptr;
}

QGraphicsObject* CommandEntry::mainTextItem() const
{
    return m_commandItem;
}

void CommandEntry::collapseResults()
{
    if (m_resultsCollapsed)
        return;

    for(auto* item : m_informationItems) {
        fadeOutItem(item, nullptr);
        item->hide();
    }

    for(auto* item : m_resultItems) {
        fadeOutItem(item->graphicsObject(), nullptr);
        item->graphicsObject()->hide();
    }

    m_resultsCollapsed = true;

    if (worksheet()->animationsEnabled())
    {
        QTimer::singleShot(100, this, &CommandEntry::setMidPrompt);
        QTimer::singleShot(200, this, &CommandEntry::setHidePrompt);
    }
    else
        setHidePrompt();

    m_controlElement.isCollapsed = true;
    animateSizeChange();
}

void CommandEntry::expandResults()
{
    if(!m_resultsCollapsed)
        return;

    for(auto* item : m_informationItems) {
        fadeInItem(item, nullptr);
        item->show();
    }

    for(auto* item : m_resultItems) {
        fadeInItem(item->graphicsObject(), nullptr);
        item->graphicsObject()->show();
    }

    m_resultsCollapsed = false;

    if (worksheet()->animationsEnabled())
    {
        QTimer::singleShot(100, this, &CommandEntry::setMidPrompt);
        QTimer::singleShot(200, this, SLOT(updatePrompt()));
    }
    else
        this->updatePrompt();

    m_controlElement.isCollapsed = false;
    animateSizeChange();
}

void CommandEntry::setHidePrompt()
{
    updatePrompt(HidePrompt);
}

void CommandEntry::setMidPrompt()
{
    updatePrompt(MidPrompt);
}

void CommandEntry::changeResultCollapsingAction()
{
    if (m_resultItems.size() == 0)
        return;

    if (m_resultsCollapsed)
        expandResults();
    else
        collapseResults();
}

qreal CommandEntry::promptItemWidth()
{
    return m_promptItem->width();
}

/*!
 * called when the "Get Help" action is triggered in the context menu.
 * requests the worksheet to show the current keyword in the documentation panel.
 * the current keyword is either the currently selected text or the text under
 * the cursor if there is no selection.
 */
void CommandEntry::showHelp()
{
    QString keyword;
    KTextEditor::View *view = m_commandItem->view();
    KTextEditor::Document *document = view->document();
    KTextEditor::Cursor cursor = view->cursorPosition();

    KTextEditor::Range selection = view->selectionRange();
    if(selection.isValid() && !selection.isEmpty())
        keyword = document->text(selection);
    else
        keyword = document->line(cursor.line());

    if(!keyword.simplified().isEmpty())
        Q_EMIT worksheet()->requestDocumentation(keyword);
}

void CommandEntry::toggleEnabled()
{
    auto* action = static_cast<QAction*>(QObject::sender());
    if (action->isChecked())
        addToExecution();
    else
        excludeFromExecution();
}


void CommandEntry::excludeFromExecution()
{
    if (m_isExecutionEnabled == false) return;
    m_isExecutionEnabled = false;

    m_activeExecutionBackgroundColor = m_commandItem->backgroundColor();
    m_activeExecutionTextColor = m_commandItem->defaultTextColor();

    KColorScheme scheme = KColorScheme(QPalette::Inactive, KColorScheme::View);
    m_commandItem->setBackgroundColor(scheme.background(KColorScheme::AlternateBackground).color());

    m_commandItem->setDefaultTextColor(scheme.foreground(KColorScheme::InactiveText).color());
}

void CommandEntry::addToExecution()
{
    if (m_isExecutionEnabled == true) return;
    m_isExecutionEnabled = true;

    m_commandItem->setBackgroundColor(m_activeExecutionBackgroundColor);

    if (m_activeExecutionTextColor.isValid())
        m_commandItem->setDefaultTextColor(m_activeExecutionTextColor);
    else 
        m_commandItem->setDefaultTextColor(QColor());
}

bool CommandEntry::isExcludedFromExecution()
{
    return m_isExecutionEnabled == false;
}

bool CommandEntry::isResultCollapsed()
{
    return m_resultsCollapsed;
}

void CommandEntry::setVariableHighlightingEnabled(bool enabled)
{
    if (m_variableHighlightingEnabled == enabled)
        return;
    m_variableHighlightingEnabled = enabled;

    if (worksheet()->session() && m_dynamicHighlighter)
    {
        if (enabled)
            m_dynamicHighlighter->updateAllHighlights();
        else
            m_dynamicHighlighter->clearAllHighlights();
    }
}
