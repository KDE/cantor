#ifndef CANTOR_VARIABLEPREVIEW_H
#define CANTOR_VARIABLEPREVIEW_H

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

#include "cantor_export.h"

namespace Cantor
{

class CANTOR_EXPORT VariablePreviewData
{
public:
    enum class Type
    {
        Unsupported,
        Table,
        Dictionary,
        Image
    };

    struct Reference
    {
        QString variableName;
        QString displayName;
        QByteArray backendData;
        Type type{Type::Unsupported};

        bool isPreviewable() const;
        QString key() const;
    };

    struct Cell
    {
        QString value;
        QString type;
        QString size;
        Reference reference;
    };

    Type type{Type::Unsupported};
    QString typeName;
    QString dimensions;
    QStringList columnNames;
    QList<QList<Cell>> rows;
    qsizetype totalRows{0};
    qsizetype offset{0};
    bool hasMore{false};
    QByteArray imageData;
    QString mimeType;

    static bool fromJson(const QByteArray& json, const QString& variableName, VariablePreviewData* data, QString* errorMessage = nullptr);
};

class CANTOR_EXPORT VariablePreviewRequest : public QObject
{
    Q_OBJECT

public:
    explicit VariablePreviewRequest(QObject* parent = nullptr);

    const VariablePreviewData& data() const;
    QString errorMessage() const;

    void complete(VariablePreviewData data);
    void fail(const QString& message);

Q_SIGNALS:
    void finished();

private:
    VariablePreviewData m_data;
    QString m_errorMessage;
    bool m_finished{false};
};

}

#endif // CANTOR_VARIABLEPREVIEW_H
