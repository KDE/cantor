#include "luavariablemodel.h"
#include "result.h"
#include <QDebug>

LuaVariableModel::LuaVariableModel(Cantor::Session* session)
: Cantor::DefaultVariableModel(session)
{
}

void LuaVariableModel::update()
{
    if (m_expression) {
        return;
    }

    const QString introspectCommand = QLatin1String(
        "vars = ''\n"
        "funcs = ''\n"
        "for name, value in pairs(_G) do\n"
        "  local type_of_value = type(value)\n"
        "  if type_of_value == 'function' then\n"
        "    funcs = funcs .. name .. '\\030'\n"
    "  elseif type(name) == 'string' and not name:match('^_') then\n"
    "    vars = vars .. name .. '\\030'\n"
    "  end\n"
    "end\n"
    "print(vars .. '\\031' .. funcs)\n"
    );

    m_expression = session()->evaluateExpression(introspectCommand, Cantor::Expression::FinishingBehavior::DoNotDelete, true);
    connect(m_expression, &Cantor::Expression::statusChanged, this, &LuaVariableModel::parseResult);
}

void LuaVariableModel::parseResult(Cantor::Expression::Status status)
{
    if (status != Cantor::Expression::Status::Done || !m_expression || !m_expression->result()) {
        if (m_expression) {
            m_expression->deleteLater();
            m_expression = nullptr;
        }
        return;
    }

    QString data = m_expression->result()->data().toString();

    int lastPromptIndex = data.lastIndexOf(QLatin1Char('>'));
    if (lastPromptIndex != -1) {
        data = data.mid(lastPromptIndex + 1);
    }

    data = data.trimmed();

    const QStringList parts = data.split(QChar(31), Qt::KeepEmptyParts);

    if (parts.size() >= 2) {
        const QStringList variables = parts[0].split(QChar(30), Qt::SkipEmptyParts);
        const QStringList functions = parts[1].split(QChar(30), Qt::SkipEmptyParts);

        QList<Variable> newVariables;
        for (const QString& name : variables) {
            newVariables.append(Variable(name.trimmed(), QString()));
        }

        setVariables(newVariables);
        setFunctions(functions);
    }

    setInitiallyPopulated();

    m_expression->deleteLater();
    m_expression = nullptr;
}
