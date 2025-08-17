#include "sagevariablemodel.h"
#include "result.h"

#include <QDebug>

SageVariableModel::SageVariableModel(Cantor::Session* session)
: Cantor::DefaultVariableModel(session)
{
}

SageVariableModel::~SageVariableModel()
{
    if (m_expression) {
        m_expression->setFinishingBehavior(Cantor::Expression::DeleteOnFinish);
    }
}

void SageVariableModel::update()
{
    if (m_expression) {
        return;
    }

    const QString introspectCommand = QLatin1String(
        "import types\n"
        "__cantor_vars__ = ''\n"
        "__cantor_funcs__ = ''\n"
        "for __cantor_name__ in dir():\n"
        "  if not __cantor_name__.startswith('_'):\n"
        "    __cantor_obj__ = globals()[__cantor_name__]\n"
        "    if isinstance(__cantor_obj__, (types.FunctionType, types.BuiltinFunctionType, types.MethodType)) or hasattr(__cantor_obj__, '__call__'):\n"
        "      __cantor_funcs__ += __cantor_name__ + u'\\030'\n"
        "    else:\n"
        "      __cantor_vars__ += __cantor_name__ + u'\\030'\n"
        "print(__cantor_vars__ + u'\\031' + __cantor_funcs__)\n"
        "del __cantor_vars__, __cantor_funcs__, __cantor_name__, __cantor_obj__\n"
    );

    m_expression = session()->evaluateExpression(introspectCommand, Cantor::Expression::FinishingBehavior::DoNotDelete, true);
    connect(m_expression, &Cantor::Expression::statusChanged, this, &SageVariableModel::parseResult);
}

void SageVariableModel::parseResult(Cantor::Expression::Status status)
{
    if (status != Cantor::Expression::Status::Done || !m_expression) {
        if (status != Cantor::Expression::Status::Computing && status != Cantor::Expression::Queued) {
            m_expression->deleteLater();
            m_expression = nullptr;
        }
        return;
    }

    auto* result = m_expression->result();
    if (!result) {
        m_expression->deleteLater();
        m_expression = nullptr;
        return;
    }

    const QString data = result->data().toString().trimmed();
    const QStringList parts = data.split(QChar(31)); // Unit Separator

    if (parts.size() >= 2) {
        const QStringList variables = parts[0].split(QChar(30), Qt::SkipEmptyParts);
        const QStringList functions = parts[1].split(QChar(30), Qt::SkipEmptyParts);

        QList<Variable> newVariables;
        newVariables.reserve(variables.size());
        for (const QString& name : variables) {
            newVariables.append(Variable(name, QString()));
        }

        setVariables(newVariables);
        setFunctions(functions);
    }

    setInitiallyPopulated();

    m_expression->deleteLater();
    m_expression = nullptr;
}
