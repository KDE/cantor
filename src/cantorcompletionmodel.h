#ifndef CANTOR_COMPLETION_MODEL_H
#define CANTOR_COMPLETION_MODEL_H

#include "lib/expression.h"

#include <KTextEditor/CodeCompletionModel>
#include <KTextEditor/CodeCompletionModelControllerInterface>
#include <KTextEditor/Range>
#include <QPointer>

namespace Cantor {
    class Session;
}

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

    explicit CantorCompletionModel(Cantor::Session* session, QObject* parent = nullptr);
    ~CantorCompletionModel() override;

    void completionInvoked(KTextEditor::View*, const KTextEditor::Range&, InvocationType) override;
    void executeCompletionItem(KTextEditor::View*, const KTextEditor::Range&, const QModelIndex&) const override;
    void executeCompletionItem(KTextEditor::View* view, int row);
    QVariant data(const QModelIndex&, int) const override;

    KTextEditor::Range completionRange(KTextEditor::View*, const KTextEditor::Cursor&) override;
    bool shouldStartCompletion(KTextEditor::View*, const QString&, bool, const KTextEditor::Cursor&) override;
    void updateCompletion(KTextEditor::View*);
    void abortCompletion() const;

Q_SIGNALS:
    void modelIsReady(const QList<CantorCompletionModel::CompletionItem>&);

private Q_SLOTS:
    void startCompletionRequest();

private:
    void handleMemberCompletionResult(Cantor::Expression*, Cantor::Expression::Status);
    bool isCompletionRequestCurrent() const;
    void insertCompletionItem(KTextEditor::View*, int row) const;

    Cantor::Session* m_session{nullptr};

    QList<CompletionItem> m_matches;
    QString m_completionContextObject;

    QTimer* m_debounceTimer{nullptr};
    QPointer<KTextEditor::View> m_pendingView;
    QPointer<KTextEditor::Document> m_pendingDocument;
    KTextEditor::Cursor m_pendingCursor;
    qint64 m_pendingRevision{-1};
    mutable quint64 m_requestId{0};
    QMetaObject::Connection m_memberCompletionConnection;
};

#endif // CANTOR_COMPLETION_MODEL_H
