#ifndef KALGEBRAVARIABLEMODEL_H
#define KALGEBRAVARIABLEMODEL_H

#include "defaultvariablemodel.h"

namespace Analitza {
    class VariablesModel;
}
class OperatorsModel;


class KAlgebraVariableModel : public Cantor::DefaultVariableModel
{
public:
    KAlgebraVariableModel(Analitza::VariablesModel* analitzaVars, OperatorsModel* analitzaFuncs, Cantor::Session* session);

    void update() override;

private:
    Analitza::VariablesModel* m_analitzaVariables;
    OperatorsModel* m_analitzaFunctions;
};

#endif // KALGEBRAVARIABLEMODEL_H
