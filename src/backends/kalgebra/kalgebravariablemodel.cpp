#include "kalgebravariablemodel.h"

#include "backend.h"
#include <analitza/analyzer.h>
#include <analitza/list.h>
#include <analitza/matrix.h>
#include <analitza/object.h>
#include <analitza/vector.h>
#include <analitzagui/variablesmodel.h>
#include <analitzagui/operatorsmodel.h>
#include"kalgebrasession.h"

#include <KLocalizedString>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

#include <utility>

namespace
{

bool isSequence(const Analitza::Object* object)
{
    return object && (object->type() == Analitza::Object::vector || object->type() == Analitza::Object::list);
}

QString objectTypeName(const Analitza::Object* object)
{
    if (!object)
        return QStringLiteral("unknown");
    switch (object->type())
    {
        case Analitza::Object::vector:
            return QStringLiteral("vector");
        case Analitza::Object::list:
            return QStringLiteral("list");
        case Analitza::Object::matrix:
            return QStringLiteral("matrix");
        case Analitza::Object::value:
            return QStringLiteral("number");
        default:
            return QStringLiteral("expression");
    }
}

int sequenceSize(const Analitza::Object* object)
{
    if (object->type() == Analitza::Object::vector)
        return static_cast<const Analitza::Vector*>(object)->size();
    return static_cast<const Analitza::List*>(object)->size();
}

const Analitza::Object* sequenceItem(const Analitza::Object* object, int index)
{
    if (object->type() == Analitza::Object::vector)
        return static_cast<const Analitza::Vector*>(object)->at(index);
    return static_cast<const Analitza::List*>(object)->at(index);
}

QByteArray kalgebraReferenceData(const QJsonArray& path)
{
    QJsonObject object;
    object.insert(QLatin1String("path"), path);
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

Cantor::VariablePreviewData::Cell kalgebraCell(const QString& value, const QString& type = QString())
{
    Cantor::VariablePreviewData::Cell cell;
    cell.value = value;
    cell.type = type;
    return cell;
}

Cantor::VariablePreviewData kalgebraPreviewData(const Analitza::Object* object, const QString& variableName, const QString& displayName, const QJsonArray& path, qsizetype offset, qsizetype limit)
{
    using PreviewData = Cantor::VariablePreviewData;
    PreviewData data;
    data.type = PreviewData::Type::Table;
    data.typeName = objectTypeName(object);
    data.offset = offset;

    if (object->type() == Analitza::Object::matrix)
    {
        const auto* matrix = static_cast<const Analitza::Matrix*>(object);
        data.totalRows = matrix->rowCount();
        const qsizetype columnCount = matrix->columnCount();
        data.dimensions = QStringLiteral("%1x%2").arg(data.totalRows).arg(columnCount);
        data.columnNames.append(QStringLiteral("@index"));
        for (qsizetype column = 0; column < columnCount; ++column)
            data.columnNames.append(QString::number(column + 1));
        for (qsizetype row = offset; row < qMin(offset + limit, data.totalRows); ++row)
        {
            QList<PreviewData::Cell> cells{kalgebraCell(QString::number(row + 1), QStringLiteral("number"))};
            for (qsizetype column = 0; column < columnCount; ++column)
            {
                const auto* item = matrix->at(row, column);
                cells.append(kalgebraCell(item ? item->toString() : QString(), objectTypeName(item)));
            }
            data.rows.append(std::move(cells));
        }
    }
    else
    {
        data.totalRows = sequenceSize(object);
        data.dimensions = QString::number(data.totalRows);
        data.columnNames = {QStringLiteral("@index"), QStringLiteral("@type"), QStringLiteral("@value")};
        for (qsizetype index = offset; index < qMin(offset + limit, data.totalRows); ++index)
        {
            const Analitza::Object* item = sequenceItem(object, index);
            PreviewData::Cell valueCell = kalgebraCell(item ? item->toString() : QString(), objectTypeName(item));
            if (isSequence(item) || (item && item->type() == Analitza::Object::matrix))
            {
                QJsonArray childPath = path;
                childPath.append(index);
                valueCell.reference.variableName = variableName;
                valueCell.reference.displayName = QStringLiteral("%1[%2]").arg(displayName).arg(index + 1);
                valueCell.reference.backendData = kalgebraReferenceData(childPath);
                valueCell.reference.type = PreviewData::Type::Table;
            }
            data.rows.append({kalgebraCell(QString::number(index + 1), QStringLiteral("number")), kalgebraCell(objectTypeName(item)), std::move(valueCell)});
        }
    }
    data.hasMore = offset + limit < data.totalRows;
    return data;
}

}

KAlgebraVariableModel::KAlgebraVariableModel(Analitza::VariablesModel* analitzaVars, OperatorsModel* analitzaFuncs, Cantor::Session* session)
: Cantor::DefaultVariableModel(session), m_analitzaVariables(analitzaVars), m_analitzaFunctions(analitzaFuncs)
{
    if (m_analitzaVariables)
    {
        m_analitzaVariables->updateInformation();
        for (int row = 0; row < m_analitzaVariables->rowCount(QModelIndex()); ++row)
            m_initialVariables.insert(m_analitzaVariables->data(m_analitzaVariables->index(row, 0)).toString());
    }
}

void KAlgebraVariableModel::update()
{
    if (!m_analitzaVariables || !m_analitzaFunctions) 
        return;

    m_analitzaVariables->updateInformation();
    KAlgebraSession* kalgebraSession = static_cast<KAlgebraSession*>(session());
    m_analitzaFunctions->setVariables(kalgebraSession->analyzer()->variables());


    QList<Variable> newVariables;
    for (int i = 0; i < m_analitzaVariables->rowCount(QModelIndex()); ++i) {
        QModelIndex nameIndex = m_analitzaVariables->index(i, 0);
        QModelIndex valueIndex = m_analitzaVariables->index(i, 1);
        if (nameIndex.isValid() && valueIndex.isValid()) {
            QString name = m_analitzaVariables->data(nameIndex).toString();
            QString value = m_analitzaVariables->data(valueIndex).toString();
            if (!m_initialVariables.contains(name))
                newVariables.append(Variable(name, value));
        }
    }

    QStringList newFunctions;
    for (int i = 0; i < m_analitzaFunctions->rowCount(QModelIndex()); ++i) {
        QModelIndex nameIndex = m_analitzaFunctions->index(i, 0);
        if (nameIndex.isValid()) 
            newFunctions.append(m_analitzaFunctions->data(nameIndex).toString());
    }

    setVariables(newVariables);
    setFunctions(newFunctions);
    setInitiallyPopulated();
}

Cantor::VariablePreviewData::Reference KAlgebraVariableModel::variablePreview(const QModelIndex& index) const
{
    auto reference = DefaultVariableModel::variablePreview(index);
    if (!index.isValid() || !m_analitzaVariables)
        return reference;

    const auto analitzaVariables = m_analitzaVariables->variables();
    const Analitza::Object* object = analitzaVariables ? analitzaVariables->value(reference.variableName) : nullptr;
    if (isSequence(object) || (object && object->type() == Analitza::Object::matrix))
    {
        reference.type = Cantor::VariablePreviewData::Type::Table;
        reference.backendData = kalgebraReferenceData({});
    }
    return reference;
}

Cantor::VariablePreviewRequest* KAlgebraVariableModel::requestVariablePreview(const Cantor::VariablePreviewData::Reference& reference, qsizetype offset, qsizetype limit, QObject* parent)
{
    auto* request = new Cantor::VariablePreviewRequest(parent);
    const auto analitzaVariables = m_analitzaVariables ? m_analitzaVariables->variables() : QSharedPointer<Analitza::Variables>();
    const Analitza::Object* object = analitzaVariables ? analitzaVariables->value(reference.variableName) : nullptr;
    const QJsonArray path = QJsonDocument::fromJson(reference.backendData).object().value(QLatin1String("path")).toArray();
    for (const auto& pathValue : path)
    {
        const int index = pathValue.toInt(-1);
        object = isSequence(object) && index >= 0 && index < sequenceSize(object) ? sequenceItem(object, index) : nullptr;
    }

    if (!object || (!isSequence(object) && object->type() != Analitza::Object::matrix))
    {
        QTimer::singleShot(0, request, [request]() {
            request->fail(i18n("The variable or nested value no longer exists."));
        });
        return request;
    }

    auto data = kalgebraPreviewData(object, reference.variableName, reference.displayName, path, qMax<qsizetype>(0, offset), qMax<qsizetype>(1, limit));
    QTimer::singleShot(0, request, [request, data = std::move(data)]() mutable {
        request->complete(std::move(data));
    });
    return request;
}
