#ifndef KALGEBRAVARIABLEMODEL_H
#define KALGEBRAVARIABLEMODEL_H

#include "defaultvariablemodel.h"

#include <QSet>

namespace Analitza {
    class VariablesModel;
}
class OperatorsModel;


class KAlgebraVariableModel : public Cantor::DefaultVariableModel
{
public:
    KAlgebraVariableModel(Analitza::VariablesModel* analitzaVars, OperatorsModel* analitzaFuncs, Cantor::Session* session);

    void update() override;
    Cantor::VariablePreviewData::Reference variablePreview(const QModelIndex& index) const override;
    Cantor::VariablePreviewRequest* requestVariablePreview(const Cantor::VariablePreviewData::Reference& reference, qsizetype offset, qsizetype limit, QObject* parent = nullptr) override;

private:
    Analitza::VariablesModel* m_analitzaVariables{nullptr};
    OperatorsModel* m_analitzaFunctions{nullptr};
    QSet<QString> m_initialVariables;
};

#endif // KALGEBRAVARIABLEMODEL_H
