#include "variablepreview.h"

#include <KLocalizedString>

#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include <utility>

using namespace Cantor;

bool VariablePreviewData::Reference::isPreviewable() const
{
    return type != Type::Unsupported;
}

QString VariablePreviewData::Reference::key() const
{
    const QByteArray data = variableName.toUtf8() + '\0' + backendData;
    return QString::fromLatin1(QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());
}

bool VariablePreviewData::fromJson(const QByteArray& json, const QString& variableName, VariablePreviewData* data, QString* errorMessage)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(json, &parseError);
    if (!data || parseError.error != QJsonParseError::NoError || !document.isObject())
    {
        if (errorMessage)
            *errorMessage = parseError.errorString();
        return false;
    }

    const QJsonObject object = document.object();
    if (object.contains(QLatin1String("errorCode")))
    {
        const QString errorCode = object.value(QLatin1String("errorCode")).toString();
        if (errorMessage)
        {
            if (errorCode == QLatin1String("unsupportedDimensions"))
            {
                const QString dimensions = object.value(QLatin1String("dimensions")).toString();
                *errorMessage = dimensions.isEmpty() ? i18n("Preview is only available for one- and two-dimensional structures.") : i18n("Preview is only available for one- and two-dimensional structures. The current dimensions are %1.", dimensions);
            }
            else if (errorCode == QLatin1String("unsupportedType"))
                *errorMessage = i18n("Preview is not supported for this variable type.");
            else if (errorCode == QLatin1String("valueMissing"))
                *errorMessage = i18n("The variable or nested value no longer exists.");
            else if (errorCode == QLatin1String("invalidReference"))
                *errorMessage = i18n("The variable preview reference is invalid.");
            else
                *errorMessage = i18n("The backend could not create a preview.");
        }
        return false;
    }

    if (object.contains(QLatin1String("error")))
    {
        if (errorMessage)
            *errorMessage = object.value(QLatin1String("error")).toString();
        return false;
    }

    const int type = object.value(QLatin1String("type")).toInt();
    if (type < static_cast<int>(Type::Unsupported) || type > static_cast<int>(Type::Image))
    {
        if (errorMessage)
            *errorMessage = i18n("The backend returned an invalid preview type.");
        return false;
    }

    VariablePreviewData parsedData;
    parsedData.type = static_cast<Type>(type);
    parsedData.typeName = object.value(QLatin1String("typeName")).toString();
    parsedData.dimensions = object.value(QLatin1String("dimensions")).toString();
    parsedData.totalRows = object.value(QLatin1String("totalRows")).toInteger();
    parsedData.offset = object.value(QLatin1String("offset")).toInteger();
    parsedData.hasMore = object.value(QLatin1String("hasMore")).toBool();
    parsedData.imageData = QByteArray::fromBase64(object.value(QLatin1String("imageData")).toString().toLatin1());
    parsedData.mimeType = object.value(QLatin1String("mimeType")).toString();

    const QJsonArray columns = object.value(QLatin1String("columns")).toArray();
    for (const auto& column : columns)
        parsedData.columnNames.append(column.toString());

    const QJsonArray rows = object.value(QLatin1String("rows")).toArray();
    for (const auto& rowValue : rows)
    {
        QList<Cell> row;
        for (const auto& cellValue : rowValue.toArray())
        {
            const QJsonObject cellObject = cellValue.toObject();
            Cell cell;
            cell.value = cellObject.value(QLatin1String("value")).toString();
            cell.type = cellObject.value(QLatin1String("valueType")).toString();
            cell.size = cellObject.value(QLatin1String("size")).toString();

            const QJsonObject referenceObject = cellObject.value(QLatin1String("reference")).toObject();
            if (!referenceObject.isEmpty())
            {
                const int referenceType = referenceObject.value(QLatin1String("type")).toInt();
                if (referenceType > static_cast<int>(Type::Unsupported) && referenceType <= static_cast<int>(Type::Image))
                {
                    cell.reference.variableName = variableName;
                    cell.reference.displayName = referenceObject.value(QLatin1String("displayName")).toString();
                    cell.reference.backendData = referenceObject.value(QLatin1String("backendData")).toString().toUtf8();
                    cell.reference.type = static_cast<Type>(referenceType);
                }
            }
            row.append(std::move(cell));
        }
        parsedData.rows.append(std::move(row));
    }

    *data = std::move(parsedData);
    return true;
}

VariablePreviewRequest::VariablePreviewRequest(QObject* parent)
    : QObject(parent)
{
}

const VariablePreviewData& VariablePreviewRequest::data() const
{
    return m_data;
}

QString VariablePreviewRequest::errorMessage() const
{
    return m_errorMessage;
}

void VariablePreviewRequest::complete(VariablePreviewData data)
{
    if (m_finished)
        return;

    m_finished = true;
    m_data = std::move(data);
    Q_EMIT finished();
}

void VariablePreviewRequest::fail(const QString& message)
{
    if (m_finished)
        return;

    m_finished = true;
    m_errorMessage = message;
    Q_EMIT finished();
}
