#ifndef CANTOR_SYMBOLMANAGER_H
#define CANTOR_SYMBOLMANAGER_H

#include <QString>
#include <QStringList>
#include <QHash>
#include <QSet>
#include "cantor_export.h"

class CANTOR_EXPORT KeywordsManager
{
public:
    explicit KeywordsManager(const QString& syntaxDefinitionName);

    const QSet<QString>& symbolList(const QString& listName) const;

    QStringList symbolLists() const;

private:
    QHash<QString, QSet<QString>> m_symbolSets;
};

#endif // CANTOR_SYMBOLMANAGER_H
