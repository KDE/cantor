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

namespace
{
QString octaveStringLiteral(QString value)
{
    value.replace(QLatin1Char('\''), QLatin1String("''"));
    return QLatin1Char('\'') + value + QLatin1Char('\'');
}
}

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
                const auto& elements = data.split(QLatin1String("\n"), Qt::SkipEmptyParts);
                int count = elements.count();
                if (count < 6)
                    continue;

                const QString& name = elements.constFirst();

                // skip the output of results that are not assigned to any variable ("ans" used by Octave)
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
                vars << Variable(name, std::move(value), size.toULongLong(), type, rows + QStringLiteral("x") + columns);
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

Cantor::VariablePreviewData::Reference OctaveVariableModel::variablePreview(const QModelIndex& index) const
{
    auto reference = DefaultVariableModel::variablePreview(index);
    if (!index.isValid())
        return reference;

    const QString type = index.siblingAtColumn(2).data().toString();
    const QStringList dimensions = index.siblingAtColumn(4).data().toString().split(QLatin1Char('x'));
    const qlonglong rows = dimensions.value(0).toLongLong();
    const qlonglong columns = dimensions.value(1).toLongLong();
    if (type.contains(QLatin1String("struct")) || type == QLatin1String("object"))
        reference.type = VariablePreviewData::Type::Dictionary;
    else if (type == QLatin1String("cell") || (rows > 0 && columns > 0 && rows * columns > 1))
        reference.type = VariablePreviewData::Type::Table;

    if (reference.isPreviewable())
        reference.backendData = reference.variableName.toUtf8();
    return reference;
}

Cantor::VariablePreviewRequest* OctaveVariableModel::requestVariablePreview(const Cantor::VariablePreviewData::Reference& reference, qsizetype offset, qsizetype limit, QObject* parent)
{
    if (!reference.isPreviewable() || reference.backendData.isEmpty())
        return DefaultVariableModel::requestVariablePreview(reference, offset, limit, parent);

    static const QString commandTemplate = QStringLiteral(R"OCTAVE(
    function __cantor_preview_kind__ = __cantor_preview_kind(__cantor_preview_value__)
      if (isstruct(__cantor_preview_value__) || isa(__cantor_preview_value__, 'containers.Map'))
        __cantor_preview_kind__ = 2;
      elseif ((iscell(__cantor_preview_value__) || isnumeric(__cantor_preview_value__) || islogical(__cantor_preview_value__)) && ndims(__cantor_preview_value__) <= 2 && (iscell(__cantor_preview_value__) || numel(__cantor_preview_value__) > 1))
        __cantor_preview_kind__ = 1;
      else
        __cantor_preview_kind__ = 0;
      endif
    endfunction

    function __cantor_preview_text__ = __cantor_preview_text(__cantor_preview_value__)
      try
        __cantor_preview_text__ = strtrim(evalc('disp(__cantor_preview_value__)'));
        __cantor_preview_text__ = strrep(__cantor_preview_text__, "\n", ' ');
        if (length(__cantor_preview_text__) > 200)
          __cantor_preview_text__ = [__cantor_preview_text__(1:200) '...'];
        endif
      catch
        __cantor_preview_text__ = '<unprintable>';
      end_try_catch
    endfunction

    function __cantor_preview_cell__ = __cantor_preview_cell(__cantor_preview_value__, __cantor_preview_expression__, __cantor_preview_name__)
      __cantor_preview_cell__ = struct('value', __cantor_preview_text(__cantor_preview_value__), 'valueType', typeinfo(__cantor_preview_value__), 'size', num2str(sizeof(__cantor_preview_value__)));
      __cantor_preview_type__ = __cantor_preview_kind(__cantor_preview_value__);
      if (__cantor_preview_type__ != 0 && !isempty(__cantor_preview_expression__))
        __cantor_preview_cell__.reference = struct('displayName', __cantor_preview_name__, 'backendData', __cantor_preview_expression__, 'type', __cantor_preview_type__);
      endif
    endfunction

    try
      __cantor_preview_expression__ = %1;
      __cantor_preview_value__ = __cantor_preview_expression__;
      __cantor_preview_offset__ = %2;
      __cantor_preview_limit__ = %3;
      __cantor_preview_type__ = __cantor_preview_kind(__cantor_preview_value__);
      __cantor_preview_result__ = struct('type', __cantor_preview_type__, 'typeName', typeinfo(__cantor_preview_value__), 'dimensions', '', 'columns', {{}}, 'rows', {{}}, 'offset', __cantor_preview_offset__, 'totalRows', 0, 'hasMore', false);

      if (__cantor_preview_type__ == 2)
        if (isstruct(__cantor_preview_value__))
          __cantor_preview_keys__ = fieldnames(__cantor_preview_value__);
          __cantor_preview_values__ = struct2cell(__cantor_preview_value__);
          __cantor_preview_child_prefix__ = ['struct2cell(' %4 '){'];
        else
          __cantor_preview_keys__ = keys(__cantor_preview_value__);
          __cantor_preview_values__ = values(__cantor_preview_value__);
          __cantor_preview_child_prefix__ = ['values(' %4 '){'];
        endif
        __cantor_preview_total__ = numel(__cantor_preview_keys__);
        __cantor_preview_result__.columns = {'@key', '@type', '@value'};
        __cantor_preview_result__.dimensions = num2str(__cantor_preview_total__);
        __cantor_preview_rows__ = {};
        for __cantor_preview_index__ = (__cantor_preview_offset__ + 1):min(__cantor_preview_offset__ + __cantor_preview_limit__, __cantor_preview_total__)
          __cantor_preview_key__ = __cantor_preview_keys__{__cantor_preview_index__};
          __cantor_preview_item__ = __cantor_preview_values__{__cantor_preview_index__};
          __cantor_preview_child_expression__ = [__cantor_preview_child_prefix__ num2str(__cantor_preview_index__) '}'];
          __cantor_preview_child_name__ = [%5 '[' __cantor_preview_text(__cantor_preview_key__) ']'];
          __cantor_preview_rows__{end + 1} = {struct('value', __cantor_preview_text(__cantor_preview_key__)), struct('value', typeinfo(__cantor_preview_item__)), __cantor_preview_cell(__cantor_preview_item__, __cantor_preview_child_expression__, __cantor_preview_child_name__)};
        endfor
        __cantor_preview_result__.rows = __cantor_preview_rows__;
        __cantor_preview_result__.totalRows = __cantor_preview_total__;
        __cantor_preview_result__.hasMore = __cantor_preview_offset__ + __cantor_preview_limit__ < __cantor_preview_total__;
      elseif (__cantor_preview_type__ == 1)
        [__cantor_preview_row_count__, __cantor_preview_column_count__] = size(__cantor_preview_value__);
        __cantor_preview_total__ = __cantor_preview_row_count__;
        __cantor_preview_rows__ = {};
        if (__cantor_preview_row_count__ == 1 || __cantor_preview_column_count__ == 1)
          __cantor_preview_total__ = numel(__cantor_preview_value__);
          __cantor_preview_result__.columns = {'@index', '@type', '@value'};
          for __cantor_preview_index__ = (__cantor_preview_offset__ + 1):min(__cantor_preview_offset__ + __cantor_preview_limit__, __cantor_preview_total__)
            if (iscell(__cantor_preview_value__))
              __cantor_preview_item__ = __cantor_preview_value__{__cantor_preview_index__};
              __cantor_preview_child_expression__ = ['(' %4 '){' num2str(__cantor_preview_index__) '}'];
            else
              __cantor_preview_item__ = __cantor_preview_value__(__cantor_preview_index__);
              __cantor_preview_child_expression__ = '';
            endif
            __cantor_preview_child_name__ = [%5 '[' num2str(__cantor_preview_index__) ']'];
            __cantor_preview_rows__{end + 1} = {struct('value', num2str(__cantor_preview_index__)), struct('value', typeinfo(__cantor_preview_item__)), __cantor_preview_cell(__cantor_preview_item__, __cantor_preview_child_expression__, __cantor_preview_child_name__)};
          endfor
          __cantor_preview_result__.dimensions = num2str(__cantor_preview_total__);
        else
          __cantor_preview_result__.columns = [{'@index'}, arrayfun(@num2str, 1:__cantor_preview_column_count__, 'UniformOutput', false)];
          for __cantor_preview_row__ = (__cantor_preview_offset__ + 1):min(__cantor_preview_offset__ + __cantor_preview_limit__, __cantor_preview_total__)
            __cantor_preview_output_row__ = {struct('value', num2str(__cantor_preview_row__))};
            for __cantor_preview_column__ = 1:__cantor_preview_column_count__
              if (iscell(__cantor_preview_value__))
                __cantor_preview_item__ = __cantor_preview_value__{__cantor_preview_row__, __cantor_preview_column__};
                __cantor_preview_child_expression__ = ['(' %4 '){' num2str(__cantor_preview_row__) ',' num2str(__cantor_preview_column__) '}'];
              else
                __cantor_preview_item__ = __cantor_preview_value__(__cantor_preview_row__, __cantor_preview_column__);
                __cantor_preview_child_expression__ = '';
              endif
              __cantor_preview_child_name__ = [%5 '[' num2str(__cantor_preview_row__) ',' num2str(__cantor_preview_column__) ']'];
              __cantor_preview_output_row__{end + 1} = __cantor_preview_cell(__cantor_preview_item__, __cantor_preview_child_expression__, __cantor_preview_child_name__);
            endfor
            __cantor_preview_rows__{end + 1} = __cantor_preview_output_row__;
          endfor
          __cantor_preview_result__.dimensions = [num2str(__cantor_preview_row_count__) 'x' num2str(__cantor_preview_column_count__)];
        endif
        __cantor_preview_result__.rows = __cantor_preview_rows__;
        __cantor_preview_result__.totalRows = __cantor_preview_total__;
        __cantor_preview_result__.hasMore = __cantor_preview_offset__ + __cantor_preview_limit__ < __cantor_preview_total__;
      else
        __cantor_preview_result__ = struct('errorCode', 'unsupportedType');
      endif
    catch __cantor_preview_exception__
      __cantor_preview_result__ = struct('error', __cantor_preview_exception__.message);
    end_try_catch

    disp(['__CANTOR_VARIABLE_PREVIEW__' base64_encode(uint8(jsonencode(__cantor_preview_result__)))]);
    clear __cantor_preview_kind __cantor_preview_text __cantor_preview_cell;
    clear __cantor_preview_*;
    )OCTAVE");

    const QString expression = QString::fromUtf8(reference.backendData);
    const QString command = commandTemplate.arg(expression, QString::number(qMax<qsizetype>(0, offset)), QString::number(qMax<qsizetype>(1, limit)), octaveStringLiteral(expression), octaveStringLiteral(reference.displayName));
    return requestVariablePreviewFromCommand(command, reference.variableName, parent);
}
