#ifndef QALCULATE_VARIABLE_MODEL_H
#define QALCULATE_VARIABLE_MODEL_H

#include "defaultvariablemodel.h"

class QalculateSession;

class QalculateVariableModel : public Cantor::DefaultVariableModel
{
    Q_OBJECT
public:
    explicit QalculateVariableModel(QalculateSession* session);
    ~QalculateVariableModel() override;

    void update() override;

private:
    QalculateSession* m_session;
};

#endif // QALCULATE_VARIABLE_MODEL_H
