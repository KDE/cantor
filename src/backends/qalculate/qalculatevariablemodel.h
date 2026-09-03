#ifndef QALCULATE_VARIABLE_MODEL_H
#define QALCULATE_VARIABLE_MODEL_H

#include "defaultvariablemodel.h"

class QalculateSession;

class QalculateVariableModel : public Cantor::DefaultVariableModel
{
    Q_OBJECT
public:
    explicit QalculateVariableModel(QalculateSession*);
    ~QalculateVariableModel() override;

    void update() override;
    Cantor::VariablePreviewData::Reference variablePreview(const QModelIndex& index) const override;
    Cantor::VariablePreviewRequest* requestVariablePreview(const Cantor::VariablePreviewData::Reference& reference, qsizetype offset, qsizetype limit, QObject* parent = nullptr) override;

private:
    QalculateSession* m_session{nullptr};
};

#endif // QALCULATE_VARIABLE_MODEL_H
