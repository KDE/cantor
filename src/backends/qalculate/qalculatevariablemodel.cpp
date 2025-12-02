#include "qalculatevariablemodel.h"
#include "qalculatesession.h"

#include <libqalculate/Calculator.h>
#include <libqalculate/Unit.h>
#include <libqalculate/Variable.h>
#include <libqalculate/Function.h>

#include <QDebug>

using namespace Cantor;

QalculateVariableModel::QalculateVariableModel(QalculateSession* session) :
DefaultVariableModel(session), m_session(session)
{
}

QalculateVariableModel::~QalculateVariableModel()
{
}

void QalculateVariableModel::update()
{
    QList<Variable> newVars;
    QStringList newFuncs;

    const auto& sessionVars = m_session->getVariables();
    for (auto it = sessionVars.constBegin(); it != sessionVars.constEnd(); ++it)
        newVars.append(Variable(it.key(), it.value()));

    if (CALCULATOR) {
        for ( auto* item : CALCULATOR->variables ) {
            QString name = QLatin1String(item->name(true).c_str());
            newVars.append(Variable(name, QString()));
        }

        for ( ExpressionItem* item : CALCULATOR->functions) 
            newFuncs << QLatin1String(item->name(true).c_str());

        for (Unit* item : CALCULATOR->units) {
            newVars.append(Variable(QLatin1String(item->name(true).c_str()), QString()));
            newVars.append(Variable(QLatin1String(item->singular().c_str()), QString()));
        }
    }

    setVariables(newVars);
    setFunctions(newFuncs);

    setInitiallyPopulated();
}
