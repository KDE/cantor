#include "kalgebravariablemodel.h"

#include "backend.h"
#include <analitza/analyzer.h>
#include <analitzagui/variablesmodel.h>
#include <analitzagui/operatorsmodel.h>
#include"kalgebrasession.h"

KAlgebraVariableModel::KAlgebraVariableModel(Analitza::VariablesModel* analitzaVars, OperatorsModel* analitzaFuncs, Cantor::Session* session)
: Cantor::DefaultVariableModel(session), m_analitzaVariables(analitzaVars), m_analitzaFunctions(analitzaFuncs)
{
}

void KAlgebraVariableModel::update()
{
    if (!m_analitzaVariables || !m_analitzaFunctions) 
        return;

    m_analitzaVariables->updateInformation();
    KAlgebraSession* kalgebraSession = static_cast<KAlgebraSession*>(session());
    m_analitzaFunctions->setVariables(kalgebraSession->analyzer()->variables());


    QList<Variable> newVariables;
    for (int i = 0; i < m_analitzaVariables->rowCount(QModelIndex()); ++i) {
        QModelIndex nameIndex = m_analitzaVariables->index(i, 0);
        QModelIndex valueIndex = m_analitzaVariables->index(i, 1);
        if (nameIndex.isValid() && valueIndex.isValid()) {
            QString name = m_analitzaVariables->data(nameIndex).toString();
            QString value = m_analitzaVariables->data(valueIndex).toString();
            newVariables.append(Variable(name, value));
        }
    }

    QStringList newFunctions;
    for (int i = 0; i < m_analitzaFunctions->rowCount(QModelIndex()); ++i) {
        QModelIndex nameIndex = m_analitzaFunctions->index(i, 0);
        if (nameIndex.isValid()) 
            newFunctions.append(m_analitzaFunctions->data(nameIndex).toString());
    }

    setVariables(newVariables);
    setFunctions(newFunctions);
    setInitiallyPopulated();
}