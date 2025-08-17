#ifndef SAGEVARIABLEMODEL_H
#define SAGEVARIABLEMODEL_H

#include "defaultvariablemodel.h"

class SageVariableModel : public Cantor::DefaultVariableModel
{
    Q_OBJECT
public:
    explicit SageVariableModel(Cantor::Session* session);
    ~SageVariableModel() override;

    void update() override;

private Q_SLOTS:
    void parseResult(Cantor::Expression::Status status);

private:
    Cantor::Expression* m_expression = nullptr;
};

#endif //SAGEVARIABLEMODEL_H
