#ifndef CANTOR_COMPLETION_MODEL_H
#define CANTOR_COMPLETION_MODEL_H

#include "lib/expression.h"

#include <KTextEditor/CodeCompletionModel>
#include <KTextEditor/CodeCompletionModelControllerInterface>
#include <KTextEditor/Range>

namespace Cantor {
    class Session;
}

class WorksheetTextEditorItem;
class QTimer;

class CantorCompletionModel : public KTextEditor::CodeCompletionModel,
public KTextEditor::CodeCompletionModelControllerInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextEditor::CodeCompletionModelControllerInterface)

public:
    struct CompletionItem {
        QString name;
        bool isFunction = false;
        bool isKeyword = false;
        bool isAttribute = false;
        bool isVariable = false;
    };

    explicit CantorCompletionModel(WorksheetTextEditorItem* parent);
    ~CantorCompletionModel() override;

    void completionInvoked(KTextEditor::View*, const KTextEditor::Range&, InvocationType) override;
    void executeCompletionItem(KTextEditor::View*, const KTextEditor::Range&, const QModelIndex&) const override;
    void executeCompletionItem(KTextEditor::View* view, int row);
    QVariant data(const QModelIndex&, int) const override;

    KTextEditor::Range completionRange(KTextEditor::View*, const KTextEditor::Cursor&) override;
    bool shouldStartCompletion(KTextEditor::View*, const QString&, bool, const KTextEditor::Cursor&) override;
    void abortCompletion();

Q_SIGNALS:
    void modelIsReady(const QList<CantorCompletionModel::CompletionItem>&);

private Q_SLOTS:
    void startCompletionRequest();
    void handleMemberCompletionResult(Cantor::Expression::Status);

private:
    Cantor::Session* m_session{nullptr};

    QList<CompletionItem> m_matches;
    QString m_completionContextObject;

    QTimer* m_debounceTimer{nullptr};
    KTextEditor::View* m_pendingView{nullptr};
    KTextEditor::Range m_pendingRange;
};

#endif // CANTOR_COMPLETION_MODEL_H
