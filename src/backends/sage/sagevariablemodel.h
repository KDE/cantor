#ifndef SAGEVARIABLEMODEL_H
#define SAGEVARIABLEMODEL_H

#include "defaultvariablemodel.h"

class SageVariableModel : public Cantor::DefaultVariableModel
{
    Q_OBJECT
public:
    explicit SageVariableModel(Cantor::Session*);
    ~SageVariableModel() override;

    void update() override;
    Cantor::VariablePreviewData::Reference variablePreview(const QModelIndex& index) const override;
    Cantor::VariablePreviewRequest* requestVariablePreview(const Cantor::VariablePreviewData::Reference& reference, qsizetype offset, qsizetype limit, QObject* parent = nullptr) override;

private Q_SLOTS:
    void parseResult(Cantor::Expression::Status);

private:
    void finishUpdate();

    Cantor::Expression* m_expression{nullptr};
    bool m_updatePending{false};
};

#endif //SAGEVARIABLEMODEL_H
