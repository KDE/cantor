/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018 Nikita Sirgienko <warquark@gmail.com>
    SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
*/

#include "octavevariablemodel.h"
#include "octavesession.h"
#include "textresult.h"

#include <QDebug>

#include "settings.h"

using namespace Cantor;

OctaveVariableModel::OctaveVariableModel(OctaveSession* session): DefaultVariableModel(session)
{
}

void OctaveVariableModel::update()
{
    static const QString code = QString::fromLatin1(
        "printf('__cantor_delimiter_line__');"
        "__cantor_list__ = who();"
        "__cantor_split_var__ = split_long_rows(0);"
        "__cantor_parse_values__ = %1;"
        "for __cantor_index__ = 1:length(__cantor_list__)"
        "  __cantor_varname__ = char(__cantor_list__{__cantor_index__});"
        "  printf([__cantor_varname__ '\\n']);"
        "  if (__cantor_parse_values__)"
        "    try"
        "      eval(['__cantor_string__ = disp(' __cantor_varname__ ');']);"
        "      printf([num2str(eval(['sizeof(' __cantor_varname__ ');'])) '\\n']);"
        "      printf([eval(['typeinfo(' __cantor_varname__ ');']) '\\n']);"
        "      printf([num2str(eval(['rows(' __cantor_varname__ ');'])) '\\n']);"
        "      printf([num2str(eval(['columns(' __cantor_varname__ ');'])) '\\n']);"
        "      printf(__cantor_string__);"
        "    catch"
        "      printf(['<unprintable value>' '\\n']);"
        "      printf(['0' '\\n']);"
        "    end_try_catch;"
        "  else"
        "    printf('');"
        "  endif;"
        "  printf('__cantor_delimiter_line__');"
        "endfor;"
        "split_long_rows(__cantor_split_var__);"
        "clear __cantor_list__;"
        "clear __cantor_index__;"
        "clear __cantor_varname__;"
        "clear __cantor_parse_values__;"
        "clear __cantor_string__;"
        "clear __cantor_split_var__;"
    );

    if (m_expr)
        return;

    const QString& cmd = code.arg(OctaveSettings::self()->variableManagement() ? QLatin1String("true") : QLatin1String("false"));
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
            const QStringList& variableData = text.split(delimiter, Qt::SkipEmptyParts);
            QList<Variable> vars;

            for (const auto& data : variableData)
            {
                // every variable data has 4 parts/elements separated by a new line - the name of the variable, its size, type and the actual value
                const auto& elements = data.split(QLatin1String("\n"), Qt::SkipEmptyParts);
                int count = elements.count();
                if (count < 6)
                    continue;

                const QString& name = elements.constFirst();

                // skip the output of results that are not assigned to any varialbe ("ans" used by Octave)
                if (name == QStringLiteral("ans"))
                    continue;

                const QString& size = elements.at(1);
                const QString& type = elements.at(2);
                const QString& rows = elements.at(3);
                const QString& columns = elements.at(4);

                // the last section(s) contain(s) the value (single-line or multi-line output)
                QString value;
                if (OctaveSettings::self()->variableManagement())
                {
                    for (int i = 5; i < count; ++i)
                    {
                        if (!value.isEmpty())
                            value += QStringLiteral("; "); // separate multi-line values like in column vectors or in matrices by a semicolon
                        value += elements.at(i).trimmed();
                    }
                }

                value = value.replace(QStringLiteral("   "), QStringLiteral(" ")); // for vectors Octave is separating the values with three blanks, replace with one
                vars << Variable(name, value, size.toULongLong(), type, rows + QStringLiteral("x") + columns);
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
