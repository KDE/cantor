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
    };

    explicit CantorCompletionModel(WorksheetTextEditorItem* parent);
    ~CantorCompletionModel() override;

    void completionInvoked(KTextEditor::View* view, const KTextEditor::Range &range, InvocationType invocationType) override;
    void executeCompletionItem(KTextEditor::View* view, const KTextEditor::Range &range, const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    KTextEditor::Range completionRange(KTextEditor::View* view, const KTextEditor::Cursor &position) override;
    bool shouldStartCompletion(KTextEditor::View* view, const QString &insertedText, bool userInsertion, const KTextEditor::Cursor &position) override;

Q_SIGNALS:
    void modelIsReady(const QList<CantorCompletionModel::CompletionItem>& matches);

private Q_SLOTS:
    void startCompletionRequest();
    void handleMemberCompletionResult(Cantor::Expression::Status status);

private:
    Cantor::Session* m_session;

    QList<CompletionItem> m_matches;
    QString m_completionContextObject;

    QTimer* m_debounceTimer;
    KTextEditor::View* m_pendingView;
    KTextEditor::Range m_pendingRange;
};

#endif // CANTOR_COMPLETION_MODEL_H
