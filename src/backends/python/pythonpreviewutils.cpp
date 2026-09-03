#include "pythonpreviewutils.h"

#include <QJsonArray>
#include <QJsonDocument>

namespace
{

QString pythonStringLiteral(const QString& value)
{
    const QByteArray json = QJsonDocument(QJsonArray{value}).toJson(QJsonDocument::Compact);
    return QString::fromUtf8(json.mid(1, json.size() - 2));
}

}

Cantor::VariablePreviewData::Type Cantor::pythonPreviewType(const QString& typeName, QString dimensions)
{
    const QString type = typeName.toLower();
    if (type.contains(QLatin1String("pil.")) && type.contains(QLatin1String("image")))
        return VariablePreviewData::Type::Image;
    if (type.contains(QLatin1String("dict")))
        return VariablePreviewData::Type::Dictionary;
    if (type.contains(QLatin1String("numpy.")) && !dimensions.isEmpty())
    {
        dimensions.remove(QLatin1Char('('));
        dimensions.remove(QLatin1Char(')'));
        dimensions.remove(QLatin1Char('['));
        dimensions.remove(QLatin1Char(']'));
        const int dimensionCount = dimensions.split(QLatin1Char(','), Qt::SkipEmptyParts).size();
        if (dimensionCount < 1 || dimensionCount > 2)
            return VariablePreviewData::Type::Unsupported;
    }
    if (type.contains(QLatin1String("list")) || type.contains(QLatin1String("tuple")) || type.contains(QLatin1String("numpy.")) || type.contains(QLatin1String("dataframe")) || type.contains(QLatin1String("matrix")) || type.contains(QLatin1String("vector")))
        return VariablePreviewData::Type::Table;
    if (type.contains(QLatin1String("freemoduleelement")))
        return VariablePreviewData::Type::Table;
    return VariablePreviewData::Type::Unsupported;
}

QByteArray Cantor::pythonPreviewReference(const QString& variableName)
{
    return QStringLiteral("globals()[%1]").arg(pythonStringLiteral(variableName)).toUtf8();
}

QString Cantor::pythonPreviewCommand(const VariablePreviewData::Reference& reference, qsizetype offset, qsizetype limit)
{
    static const QString commandTemplate = QStringLiteral(R"PY(
import base64 as __cantor_preview_base64
import io as __cantor_preview_io
import json as __cantor_preview_json
import reprlib as __cantor_preview_reprlib
import sys as __cantor_preview_sys

def __cantor_preview_kind(value):
    value_type = type(value)
    module = getattr(value_type, '__module__', '')
    name = getattr(value_type, '__name__', '')
    if module.startswith('PIL.') and hasattr(value, 'save'):
        return 3
    if isinstance(value, dict):
        return 2
    if isinstance(value, (list, tuple)):
        return 1
    if module.startswith('numpy') and hasattr(value, 'ndim'):
        return 1 if value.ndim in (1, 2) else 0
    if module.startswith('pandas') and name == 'DataFrame':
        return 1
    if module.startswith('sage.matrix.') or module.startswith('sage.modules.'):
        return 1
    return 0

def __cantor_preview_shape(value):
    shape = getattr(value, 'shape', None)
    module = type(value).__module__
    if shape is None and module.startswith('sage.matrix.'):
        shape = (value.nrows(), value.ncols())
    elif shape is None and module.startswith('sage.modules.'):
        # Sage free-module elements expose len() or degree(), depending on type/version.
        try:
            shape = (len(value),)
        except (TypeError, AttributeError):
            if hasattr(value, 'degree'):
                shape = (int(value.degree()),)
    return shape

def __cantor_preview_text(value):
    try:
        if type(value).__module__.startswith('numpy') and getattr(value, 'ndim', None) == 0 and hasattr(value, 'item'):
            value = value.item()
        return __cantor_preview_reprlib.repr(value).replace('\n', ' ')
    except Exception:
        return '<unprintable>'

def __cantor_preview_key_text(value):
    if isinstance(value, str):
        return value.replace('\r', '\\r').replace('\n', '\\n')
    return __cantor_preview_text(value)

def __cantor_preview_cell(value, expression=None, display_name=''):
    kind = __cantor_preview_kind(value)
    cell = {
        'value': __cantor_preview_text(value),
        'valueType': type(value).__name__,
        'size': str(__cantor_preview_sys.getsizeof(value))
    }
    if kind and expression:
        cell['reference'] = {
            'displayName': display_name,
            'backendData': expression,
            'type': kind
        }
    return cell

try:
    __cantor_preview_expression = %1
    __cantor_preview_value = __cantor_preview_expression
    __cantor_preview_offset = %2
    __cantor_preview_limit = %3
    __cantor_preview_type = __cantor_preview_kind(__cantor_preview_value)
    __cantor_preview_result = {
        'type': __cantor_preview_type,
        'typeName': type(__cantor_preview_value).__name__,
        'dimensions': '',
        'columns': [],
        'rows': [],
        'offset': __cantor_preview_offset,
        'totalRows': 0,
        'hasMore': False
    }

    if __cantor_preview_type == 3:
        __cantor_preview_buffer = __cantor_preview_io.BytesIO()
        __cantor_preview_value.save(__cantor_preview_buffer, format='PNG')
        __cantor_preview_result['imageData'] = __cantor_preview_base64.b64encode(__cantor_preview_buffer.getvalue()).decode('ascii')
        __cantor_preview_result['mimeType'] = 'image/png'
        __cantor_preview_result['dimensions'] = '{}x{}'.format(*__cantor_preview_value.size)
    elif __cantor_preview_type == 2:
        __cantor_preview_keys = list(__cantor_preview_value.keys())
        __cantor_preview_total = len(__cantor_preview_keys)
        __cantor_preview_result['columns'] = ['@key', '@type', '@size', '@value']
        __cantor_preview_result['totalRows'] = __cantor_preview_total
        __cantor_preview_result['dimensions'] = str(__cantor_preview_total)
        for __cantor_preview_index in range(__cantor_preview_offset, min(__cantor_preview_offset + __cantor_preview_limit, __cantor_preview_total)):
            __cantor_preview_key = __cantor_preview_keys[__cantor_preview_index]
            __cantor_preview_item = __cantor_preview_value[__cantor_preview_key]
            __cantor_preview_item_expression = 'list((' + %4 + ').values())[' + str(__cantor_preview_index) + ']'
            __cantor_preview_name = %5 + '[' + __cantor_preview_key_text(__cantor_preview_key) + ']'
            __cantor_preview_result['rows'].append([
                {'value': __cantor_preview_key_text(__cantor_preview_key)},
                {'value': type(__cantor_preview_item).__name__},
                {'value': str(__cantor_preview_sys.getsizeof(__cantor_preview_item))},
                __cantor_preview_cell(__cantor_preview_item, __cantor_preview_item_expression, __cantor_preview_name)
            ])
        __cantor_preview_result['hasMore'] = __cantor_preview_offset + __cantor_preview_limit < __cantor_preview_total
    elif __cantor_preview_type == 1:
        __cantor_preview_module = type(__cantor_preview_value).__module__
        __cantor_preview_is_dataframe = __cantor_preview_module.startswith('pandas') and type(__cantor_preview_value).__name__ == 'DataFrame'
        if __cantor_preview_is_dataframe:
            __cantor_preview_total = len(__cantor_preview_value.index)
            __cantor_preview_result['columns'] = ['@index'] + [str(column) for column in __cantor_preview_value.columns]
            __cantor_preview_result['dimensions'] = '{}x{}'.format(__cantor_preview_total, len(__cantor_preview_value.columns))
            for __cantor_preview_index in range(__cantor_preview_offset, min(__cantor_preview_offset + __cantor_preview_limit, __cantor_preview_total)):
                __cantor_preview_row = [{'value': __cantor_preview_text(__cantor_preview_value.index[__cantor_preview_index])}]
                __cantor_preview_row.extend(__cantor_preview_cell(value) for value in __cantor_preview_value.iloc[__cantor_preview_index].tolist())
                __cantor_preview_result['rows'].append(__cantor_preview_row)
        else:
            __cantor_preview_shape_value = __cantor_preview_shape(__cantor_preview_value)
            __cantor_preview_rows_are_sequences = bool(__cantor_preview_value) and all(isinstance(row, (list, tuple)) for row in __cantor_preview_value) if __cantor_preview_shape_value is None else False
            __cantor_preview_is_matrix = len(__cantor_preview_shape_value) == 2 if __cantor_preview_shape_value is not None else __cantor_preview_rows_are_sequences and len({len(row) for row in __cantor_preview_value}) == 1
            __cantor_preview_total = int(__cantor_preview_shape_value[0]) if __cantor_preview_shape_value is not None else len(__cantor_preview_value)
            if __cantor_preview_is_matrix:
                __cantor_preview_columns = int(__cantor_preview_shape_value[1]) if __cantor_preview_shape_value is not None else max((len(row) for row in __cantor_preview_value), default=0)
                __cantor_preview_result['columns'] = ['@index'] + [str(column) for column in range(__cantor_preview_columns)]
                __cantor_preview_result['dimensions'] = '{}x{}'.format(__cantor_preview_total, __cantor_preview_columns)
                for __cantor_preview_index in range(__cantor_preview_offset, min(__cantor_preview_offset + __cantor_preview_limit, __cantor_preview_total)):
                    __cantor_preview_row_value = __cantor_preview_value[__cantor_preview_index]
                    __cantor_preview_row = [{'value': str(__cantor_preview_index)}]
                    __cantor_preview_row.extend(__cantor_preview_cell(value) for value in __cantor_preview_row_value)
                    __cantor_preview_result['rows'].append(__cantor_preview_row)
            else:
                __cantor_preview_result['columns'] = ['@index', '@type', '@value']
                __cantor_preview_result['dimensions'] = str(__cantor_preview_total)
                for __cantor_preview_index in range(__cantor_preview_offset, min(__cantor_preview_offset + __cantor_preview_limit, __cantor_preview_total)):
                    __cantor_preview_item = __cantor_preview_value[__cantor_preview_index]
                    __cantor_preview_item_expression = '(' + %4 + ')[' + str(__cantor_preview_index) + ']'
                    __cantor_preview_name = %5 + '[' + str(__cantor_preview_index) + ']'
                    __cantor_preview_result['rows'].append([
                        {'value': str(__cantor_preview_index)},
                        {'value': type(__cantor_preview_item).__name__},
                        __cantor_preview_cell(__cantor_preview_item, __cantor_preview_item_expression, __cantor_preview_name)
                    ])
        __cantor_preview_result['totalRows'] = __cantor_preview_total
        __cantor_preview_result['hasMore'] = __cantor_preview_offset + __cantor_preview_limit < __cantor_preview_total
    else:
        __cantor_preview_shape_value = __cantor_preview_shape(__cantor_preview_value)
        if __cantor_preview_shape_value is not None:
            __cantor_preview_result = {
                'errorCode': 'unsupportedDimensions',
                'dimensions': ' × '.join(str(size) for size in __cantor_preview_shape_value)
            }
        else:
            __cantor_preview_result = {'errorCode': 'unsupportedType'}
except Exception as __cantor_preview_exception:
    __cantor_preview_result = {'error': str(__cantor_preview_exception)}

print('__CANTOR_VARIABLE_PREVIEW__' + __cantor_preview_base64.b64encode(__cantor_preview_json.dumps(__cantor_preview_result, default=int).encode('utf-8')).decode('ascii'))

for __cantor_preview_name in [name for name in list(globals()) if name.startswith('__cantor_preview_')]:
    del globals()[__cantor_preview_name]
)PY");

    const QString expression = QString::fromUtf8(reference.backendData);
    return commandTemplate.arg(expression, QString::number(qMax<qsizetype>(0, offset)), QString::number(qMax<qsizetype>(1, limit)), pythonStringLiteral(expression), pythonStringLiteral(reference.displayName));
}
