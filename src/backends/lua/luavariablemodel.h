#ifndef LUAVARIABLEMODEL_H
#define LUAVARIABLEMODEL_H

#include "defaultvariablemodel.h"

#include <QSet>
#include <QTemporaryFile>

#include <memory>

class LuaVariableModel : public Cantor::DefaultVariableModel
{
public:
    LuaVariableModel(Cantor::Session* session);
    ~LuaVariableModel() override;

    void initialize();
    void update() override;
    Cantor::VariablePreviewData::Reference variablePreview(const QModelIndex& index) const override;
    Cantor::VariablePreviewRequest* requestVariablePreview(const Cantor::VariablePreviewData::Reference& reference, qsizetype offset, qsizetype limit, QObject* parent = nullptr) override;

private Q_SLOTS:
    void parseResult(Cantor::Expression::Status status);

private:
    QString scriptCommand(const QString& source);
    void finishUpdate();

    Cantor::Expression* m_expression{nullptr};
    std::unique_ptr<QTemporaryFile> m_scriptFile;
    QSet<QString> m_baselineNames;
    QStringList m_baselineFunctions;
    bool m_initializing{false};
    bool m_updatePending{false};
};

#endif // LUAVARIABLEMODEL_H
