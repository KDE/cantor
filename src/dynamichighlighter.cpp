#include "dynamichighlighter.h"
#include "lib/defaultvariablemodel.h"
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <QColor>
#include <QDebug>

DynamicHighlighter::DynamicHighlighter(KTextEditor::Document* document, Cantor::DefaultVariableModel* model, const QColor& variableColor, const QColor& functionColor, QObject* parent)
: QObject(parent), m_document(document), m_variableModel(model)
{
    m_variableAttribute = KTextEditor::Attribute::Ptr(new KTextEditor::Attribute());
    m_variableAttribute->setForeground(variableColor);

    m_functionAttribute = KTextEditor::Attribute::Ptr(new KTextEditor::Attribute());
    m_functionAttribute->setForeground(functionColor);

    if (m_variableModel) {
        connect(m_variableModel, &Cantor::DefaultVariableModel::variablesAdded, this, &DynamicHighlighter::handleVariablesAdded);
        connect(m_variableModel, &Cantor::DefaultVariableModel::variablesRemoved, this, &DynamicHighlighter::handleVariablesRemoved);
        connect(m_variableModel, &Cantor::DefaultVariableModel::functionsAdded, this, &DynamicHighlighter::handleFunctionsAdded);
        connect(m_variableModel, &Cantor::DefaultVariableModel::functionsRemoved, this, &DynamicHighlighter::handleFunctionsRemoved);
    }
}

DynamicHighlighter::~DynamicHighlighter()
{
    if (m_variableModel) {
        disconnect(m_variableModel, &Cantor::DefaultVariableModel::variablesAdded, this, &DynamicHighlighter::handleVariablesAdded);
        disconnect(m_variableModel, &Cantor::DefaultVariableModel::variablesRemoved, this, &DynamicHighlighter::handleVariablesRemoved);
        disconnect(m_variableModel, &Cantor::DefaultVariableModel::functionsAdded, this, &DynamicHighlighter::handleFunctionsAdded);
        disconnect(m_variableModel, &Cantor::DefaultVariableModel::functionsRemoved, this, &DynamicHighlighter::handleFunctionsRemoved);
    }
    clearAllHighlights();
}

void DynamicHighlighter::updateAllHighlights()
{
    clearAllHighlights();
    if (m_variableModel)
    {
        handleVariablesAdded(m_variableModel->variableNames());
        handleFunctionsAdded(m_variableModel->functions());
    }
}

void DynamicHighlighter::clearAllHighlights()
{
    for (auto* range : m_highlightedVariableRanges)
    {
        delete range;
    }
    m_highlightedVariableRanges.clear();

    for (auto* range : m_highlightedFunctionRanges)
    {
        delete range;
    }
    m_highlightedFunctionRanges.clear();

    if (!m_document->views().isEmpty())
    {
        m_document->views().first()->update();
    }
}

void DynamicHighlighter::updateThemeColors(const QColor& variableColor, const QColor& functionColor)
{
    m_variableAttribute->setForeground(variableColor);
    m_functionAttribute->setForeground(functionColor);
    updateAllHighlights();
}

void DynamicHighlighter::handleVariablesAdded(const QStringList& variables)
{
    applyHighlights(variables, m_variableAttribute, m_highlightedVariableRanges);
}

void DynamicHighlighter::handleVariablesRemoved(const QStringList& variables)
{
    removeHighlights(variables, m_highlightedVariableRanges);
}

void DynamicHighlighter::handleFunctionsAdded(const QStringList& functions)
{
    applyHighlights(functions, m_functionAttribute, m_highlightedFunctionRanges);
}

void DynamicHighlighter::handleFunctionsRemoved(const QStringList& functions)
{
    removeHighlights(functions, m_highlightedFunctionRanges);
}

void DynamicHighlighter::applyHighlights(const QStringList& symbols, KTextEditor::Attribute::Ptr attribute, QList<KTextEditor::MovingRange*>& rangeList)
{
    if (!m_document || symbols.isEmpty())
    {
        return;
    }

    const KTextEditor::Range searchRange(KTextEditor::Cursor(0, 0), m_document->documentEnd());
    for (const QString& symbolName : symbols)
    {
        if (symbolName.isEmpty())
            continue;

        // Use whole-word search
        const auto occurrences = m_document->searchText(searchRange, symbolName, KTextEditor::SearchOption::WholeWords);
        for (const auto& range : occurrences)
        {
            KTextEditor::MovingRange* movingRange = m_document->newMovingRange(range);
            movingRange->setAttribute(attribute);
            movingRange->setZDepth(10.0);
            rangeList.append(movingRange);
        }
    }

    if (!m_document->views().isEmpty())
    {
        m_document->views().first()->update();
    }
}

void DynamicHighlighter::removeHighlights(const QStringList& symbols, QList<KTextEditor::MovingRange*>& rangeList)
{
    if (!m_document)
    {
        return;
    }

    QList<KTextEditor::MovingRange*> remainingRanges;
    QList<KTextEditor::MovingRange*> rangesToDelete;

    for (auto* range : rangeList)
    {
        KTextEditor::Range currentRange(*range);
        if (!currentRange.isValid())
        {
            rangesToDelete.append(range);
            continue;
        }

        if (symbols.contains(m_document->text(currentRange)))
        {
            rangesToDelete.append(range);
        }
        else
        {
            remainingRanges.append(range);
        }
    }

    rangeList = remainingRanges;

    if (!rangesToDelete.isEmpty())
    {
        qDeleteAll(rangesToDelete);

        if (!m_document->views().isEmpty())
        {
            m_document->views().first()->update();
        }
    }
}
