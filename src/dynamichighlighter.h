#ifndef CANTOR_DYNAMICHLIGHTER_H
#define CANTOR_DYNAMICHLIGHTER_H

#include <QObject>
#include <QPointer>
#include <KTextEditor/Attribute>
#include <KTextEditor/MovingRange>

namespace KTextEditor {
    class Document;
}

namespace Cantor {
    class DefaultVariableModel;
}

class DynamicHighlighter : public QObject
{
    Q_OBJECT

public:
    explicit DynamicHighlighter(KTextEditor::Document* document, Cantor::DefaultVariableModel* model, const QColor& variableColor, const QColor& functionColor, QObject* parent = nullptr);
    ~DynamicHighlighter() override;

public Q_SLOTS:
    void updateAllHighlights();
    void clearAllHighlights();
    void updateThemeColors(const QColor&, const QColor&);

private Q_SLOTS:
    void handleVariablesAdded(const QStringList&);
    void handleVariablesRemoved(const QStringList&);
    void handleFunctionsAdded(const QStringList&);
    void handleFunctionsRemoved(const QStringList&);

private:
    void applyHighlights(const QStringList& symbols, KTextEditor::Attribute::Ptr attribute, QList<KTextEditor::MovingRange*>& rangeList);
    void removeHighlights(const QStringList& symbols, QList<KTextEditor::MovingRange*>& rangeList);

    QPointer<KTextEditor::Document> m_document;
    QPointer<Cantor::DefaultVariableModel> m_variableModel;

    KTextEditor::Attribute::Ptr m_variableAttribute;
    QList<KTextEditor::MovingRange*> m_highlightedVariableRanges;

    KTextEditor::Attribute::Ptr m_functionAttribute;
    QList<KTextEditor::MovingRange*> m_highlightedFunctionRanges;
};

#endif // CANTOR_DYNAMICHLIGHTER_H
