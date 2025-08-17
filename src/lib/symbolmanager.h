#ifndef CANTOR_SYMBOLMANAGER_H
#define CANTOR_SYMBOLMANAGER_H

#include <QString>
#include <QStringList>
#include <QHash>
#include <QSet>
#include "cantor_export.h"

class CANTOR_EXPORT SymbolManager
{
public:
    explicit SymbolManager(const QString& syntaxDefinitionName);

    const QSet<QString>& getSymbolList(const QString& listName) const;

    QStringList getAvailableLists() const;

private:
    QHash<QString, QSet<QString>> m_symbolSets;
    QSet<QString> m_emptySet;
};

#endif // CANTOR_SYMBOLMANAGER_H
