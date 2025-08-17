#ifndef LUAVARIABLEMODEL_H
#define LUAVARIABLEMODEL_H

#include "defaultvariablemodel.h"

class LuaVariableModel : public Cantor::DefaultVariableModel
{
public:
    LuaVariableModel(Cantor::Session* session);

    void update() override;

private Q_SLOTS:
    void parseResult(Cantor::Expression::Status status);

private:
    Cantor::Expression* m_expression{nullptr};
};

#endif // LUAVARIABLEMODEL_H
