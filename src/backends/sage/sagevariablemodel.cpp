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
        "import sys\n"
        "import types\n"
        "__cantor_vars__ = []\n"
        "__cantor_funcs__ = []\n"
        "for __cantor_name__ in dir():\n"
        "    if not __cantor_name__.startswith('_'):\n"
        "        __cantor_obj__ = globals()[__cantor_name__]\n"
        "        if isinstance(__cantor_obj__, (types.FunctionType, types.BuiltinFunctionType, types.MethodType)) or hasattr(__cantor_obj__, '__call__'):\n"
        "            __cantor_funcs__.append(__cantor_name__)\n"
        "        else:\n"
        "            try:\n"
        "                __val_str = str(__cantor_obj__).replace('\\n', ' ')\n"
        "                if len(__val_str) > 100: __val_str = __val_str[:100] + '...'\n"
        "                __sz_str = str(sys.getsizeof(__cantor_obj__))\n"
        "                __tp_str = type(__cantor_obj__).__name__\n"
        "            except:\n"
        "                __val_str = 'Error'\n"
        "                __sz_str = '0'\n"
        "                __tp_str = 'Unknown'\n"
        "            __cantor_vars__.append(f'{__cantor_name__}\\x1d{__val_str}\\x1d{__sz_str}\\x1d{__tp_str}')\n"
        "print('\\x1e'.join(__cantor_vars__) + '\\x1f' + '\\x1e'.join(__cantor_funcs__))\n"
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
    const QStringList parts = data.split(QChar(31)); 

    if (parts.size() >= 2) {
        const QStringList variables = parts[0].split(QChar(30), Qt::SkipEmptyParts);
        const QStringList functions = parts[1].split(QChar(30), Qt::SkipEmptyParts);

        QList<Variable> newVariables;
        newVariables.reserve(variables.size());
        
        for (const QString& record : variables) {
            const QStringList fields = record.split(QChar(29));
            
            if (fields.size() >= 4) {
                newVariables.append(Variable(fields[0], fields[1], fields[2].toULongLong(), fields[3]));
            } 
            else if (!fields.isEmpty()) {
                newVariables.append(Variable(fields[0], QString()));
            }
        }

        setVariables(newVariables);
        setFunctions(functions);
    }

    setInitiallyPopulated();

    m_expression->deleteLater();
    m_expression = nullptr;
}