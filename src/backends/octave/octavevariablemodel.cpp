/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018 Nikita Sirgienko <warquark@gmail.com>
*/

#include "octavevariablemodel.h"
#include "octavesession.h"
#include "textresult.h"

#include <QDebug>

#include "settings.h"

using namespace Cantor;

OctaveVariableModel::OctaveVariableModel(OctaveSession* session):
    DefaultVariableModel(session),
    m_expr(nullptr)
{
}

void OctaveVariableModel::update()
{
    static const QString code = QString::fromLatin1(
        "printf('__cantor_delimiter_line__\\n');"
        "__cantor_list__ = who();"
        "__cantor_split_var__ = split_long_rows(0);"
        "__cantor_parse_values__ = %1;"
        "for __cantor_index__ = 1:length(__cantor_list__)"
        "  __cantor_varname__ = char(__cantor_list__{__cantor_index__});"
        "  printf([__cantor_varname__ '\\n']);"
        "  if (__cantor_parse_values__)"
        "    try"
        "      eval(['__cantor_string__ = disp(' __cantor_varname__ ');']);"
        "      printf(__cantor_string__);"
        "      printf([num2str(eval(['sizeof(' __cantor_varname__ ');'])) '\\n']);"
        "    catch"
        "      printf(['<unprintable value>' '\\n']);"
        "      printf(['0' '\\n']);"
        "    end_try_catch;"
        "  else"
        "    printf('');"
        "  endif;"
        "  printf('__cantor_delimiter_line__\\n');"
        "endfor;"
        "split_long_rows(__cantor_split_var__);"
        "clear __cantor_list__;"
        "clear __cantor_index__;"
        "clear __cantor_varname__;"
        "clear __cantor_parse_values__;"
        "clear __cantor_string__;"
        "clear __cantor_split_var__;"
    );

    const QString& cmd = code.arg(OctaveSettings::self()->variableManagement() ? QLatin1String("true") : QLatin1String("false"));

    if (m_expr)
        return;

    m_expr = session()->evaluateExpression(cmd, Expression::FinishingBehavior::DoNotDelete, true);
    connect(m_expr, &Expression::statusChanged, this, &OctaveVariableModel::parseNewVariables);
}

void OctaveVariableModel::parseNewVariables(Expression::Status status)
{
    switch(status)
    {
        case Expression::Status::Done:
        {
            static const QLatin1String delimiter("__cantor_delimiter_line__");

            if (m_expr->results().isEmpty())
            {
                qWarning() << "Octave code for parsing variables finish with done status, but without results";
                break;
            }

            QString text = static_cast<Cantor::TextResult*>(m_expr->result())->plain();
            const QStringList& lines = text.split(delimiter, QString::SkipEmptyParts);

            QList<Variable> vars;
            for (QString line : lines)
            {
                line = line.trimmed();
                const QString& name = line.section(QLatin1String("\n"), 0, 0);
                QString value;
                if (OctaveSettings::self()->variableManagement())
                    value = line.section(QLatin1String("\n"), 1, 1);
                size_t size = line.section(QLatin1String("\n"), 2, 2).toULongLong();
                vars << Variable(name, value, size);
            }

            setVariables(vars);
            break;
        }
        case Expression::Status::Error:
            qWarning() << "Octave code for parsing variables finish with error message: " << m_expr->errorMessage();
            break;

        default:
            return;
    }

    m_expr->deleteLater();
    m_expr = nullptr;
}
