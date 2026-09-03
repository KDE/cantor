#ifndef SCILABVARIABLEMODEL_H
#define SCILABVARIABLEMODEL_H

#include "defaultvariablemodel.h"

class ScilabVariableModel : public Cantor::DefaultVariableModel
{
    Q_OBJECT

public:
    explicit ScilabVariableModel(Cantor::Session* session);

    void update() override;

    Cantor::VariablePreviewData::Reference variablePreview(const QModelIndex& index) const override;
    Cantor::VariablePreviewRequest* requestVariablePreview(const Cantor::VariablePreviewData::Reference& reference, qsizetype offset, qsizetype limit, QObject* parent = nullptr) override;

private Q_SLOTS:
    void parseVariables(Cantor::Expression::Status status);

private:
    void finishUpdate();

    Cantor::Expression* m_expression{nullptr};
    bool m_updatePending{false};
};

#endif // SCILABVARIABLEMODEL_H
