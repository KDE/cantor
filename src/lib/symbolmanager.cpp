#include "symbolmanager.h"
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Definition>
#include <QDebug>

SymbolManager::SymbolManager(const QString& syntaxDefinitionName)
{
    KSyntaxHighlighting::Repository repo;
    KSyntaxHighlighting::Definition definition = repo.definitionForName(syntaxDefinitionName);

    if (!definition.isValid())
    {
        qWarning() << "SymbolManager: Could not find syntax definition for" << syntaxDefinitionName;
        return;
    }

    const QStringList keywordLists = definition.keywordLists();
    for (const QString& listName : keywordLists)
    {
        const QStringList keywords = definition.keywordList(listName);
        m_symbolSets[listName] = QSet<QString>(keywords.begin(), keywords.end());
    }
}

const QSet<QString>& SymbolManager::symbolList(const QString& listName) const
{
    auto it = m_symbolSets.constFind(listName);
    if (it != m_symbolSets.constEnd())
    {
        return it.value();
    }
    static const QSet<QString> empty;
    return empty;
}

QStringList SymbolManager::symbolLists() const
{
    return m_symbolSets.keys();
}
