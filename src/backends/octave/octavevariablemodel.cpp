/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2018 Nikita Sirgienko <warquark@gmail.com>
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
        "__cantor_parse_values__ = %1;"
        "for __cantor_index__ = 1:length(__cantor_list__)"
        "  __cantor_varname__ = char(__cantor_list__{__cantor_index__});"
        "  printf([__cantor_varname__ '\\n']);"
        "  if (__cantor_parse_values__)"
        "    try"
        "      eval(['__cantor_string__ = disp(' __cantor_varname__ ');']);"
        "      printf(__cantor_string__);"
        "    catch"
        "      printf(['<unprintable value>' '\\n']);"
        "    end_try_catch;"
        "  endif;"
        "  printf('__cantor_delimiter_line__\\n')"
        "endfor;"
        "clear __cantor_list__;"
        "clear __cantor_index__;"
        "clear __cantor_varname__;"
        "clear __cantor_parse_values__;"
        "clear __cantor_string__;"
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

            // Result always must be, if we done, so don't check it
            QString text = static_cast<Cantor::TextResult*>(m_expr->result())->plain();
            const QStringList& lines = text.split(delimiter, QString::SkipEmptyParts);

            QList<Variable> vars;
            for (QString line : lines)
            {
                line = line.trimmed();
                const QString& name = line.section(QLatin1String("\n"), 0, 0);
                QString value;
                if (OctaveSettings::self()->variableManagement())
                    value = line.section(QLatin1String("\n"), 1);
                vars << Variable{name, value};
            }

            setVariables(vars);
            break;
        }
        case Expression::Status::Error:
            qWarning() << "Octave code for parsing variables finish with error message: " << m_expr->errorMessage();

        default:
            return;
    }

    m_expr->deleteLater();
    m_expr = nullptr;
}
