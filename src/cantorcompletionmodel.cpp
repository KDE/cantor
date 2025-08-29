#include "cantorcompletionmodel.h"
#include "worksheettexteditoritem.h"
#include "worksheet.h"
#include "lib/session.h"
#include "lib/symbolmanager.h"
#include "lib/defaultvariablemodel.h"

#include <KTextEditor/View>
#include <KTextEditor/Document>
#include <QTimer>
#include <QSet>
#include <algorithm>

CantorCompletionModel::CantorCompletionModel(WorksheetTextEditorItem* parent)
: KTextEditor::CodeCompletionModel(parent),
m_session(nullptr),
m_pendingView(nullptr)
{
    if (parent && parent->worksheet())
    {
        m_session = parent->worksheet()->session();
    }

    m_debounceTimer = new QTimer(this);
    m_debounceTimer->setSingleShot(true);
    m_debounceTimer->setInterval(200);
    connect(m_debounceTimer, &QTimer::timeout, this, &CantorCompletionModel::startCompletionRequest);
}

CantorCompletionModel::~CantorCompletionModel()
{
    m_debounceTimer->stop();
}

void CantorCompletionModel::completionInvoked(KTextEditor::View* view, const KTextEditor::Range &range, InvocationType invocationType)
{
    // For more readable debug output, convert the enum to a string
    // QString invocationTypeString;
    // switch (invocationType)
    // {
    //     case KTextEditor::CodeCompletionModel::AutomaticInvocation:
    //         invocationTypeString = QStringLiteral("Automatic");
    //         break;
    //     case KTextEditor::CodeCompletionModel::UserInvocation:
    //         invocationTypeString = QStringLiteral("User");
    //         break;
    //     default:
    //         invocationTypeString = QStringLiteral("Unknown");
    //         break;
    // }
    // qDebug() << "[Completion INVOKED] InvocationType:" << invocationTypeString;

    // DEBUG: Log cursor and range details
    // KTextEditor::Cursor cursor = view->cursorPosition();
    // qDebug() << "[Completion INVOKED] Cursor Position: Line" << cursor.line() << ", Column" << cursor.column();
    // qDebug() << "[Completion INVOKED] Range: from (" << range.start().line() << "," << range.start().column()
    // << ") to (" << range.end().line() << "," << range.end().column() << ")";
    // qDebug() << "[Completion INVOKED] Text in range:" << view->document()->text(range);

    // DEBUG: Check for member access context
    // if (cursor.column() > 0)
    // {
    //     QChar precedingChar = view->document()->characterAt(KTextEditor::Cursor(cursor.line(), cursor.column() - 1));
    //     if (precedingChar == QLatin1Char('.'))
    //     {
    //         KTextEditor::Range objectRange = view->document()->wordRangeAt(KTextEditor::Cursor(cursor.line(), cursor.column() - 1));
    //         QString objectName = view->document()->text(objectRange);
    //         qDebug() << "[Completion INVOKED] Member access detected for object:" << objectName;
    //     }
    // }

    m_pendingView = view;
    m_pendingRange = range;
    m_debounceTimer->start();
}

void CantorCompletionModel::startCompletionRequest()
{
    if (!m_pendingView || !m_session)
    {
        return;
    }

    const auto* symbolManager = m_session->symbolManager();
    const auto* varModel = m_session->variableModel();
    if (!symbolManager || !varModel)
    {
        return;
    }

    QSet<QString> allSymbolsSet;
    QSet<QString> allFunctionsSet;
    QSet<QString> allKeywordsSet;

    const QStringList staticLists = symbolManager->getAvailableLists();
    for (const QString& listName : staticLists)
    {
        const QSet<QString>& symbols = symbolManager->getSymbolList(listName);
        allSymbolsSet.unite(symbols);
        if (listName.contains(QStringLiteral("func"), Qt::CaseInsensitive))
        {
            allFunctionsSet.unite(symbols);
        }
        if (listName == QStringLiteral("import") ||
            listName == QStringLiteral("flow") ||
            listName == QStringLiteral("flow_yield") ||
            listName == QStringLiteral("defs") ||
            listName == QStringLiteral("exceptions") ||
            listName == QStringLiteral("patternmatching"))
        {
            allKeywordsSet.unite(symbols);
        }
    }

    const QStringList userVariables = varModel->variableNames();
    const QStringList userFunctions = varModel->functions();

    const QSet<QString> userVariablesSet(userVariables.constBegin(), userVariables.constEnd());
    const QSet<QString> userFunctionsSet(userFunctions.constBegin(), userFunctions.constEnd());

    allSymbolsSet.unite(userVariablesSet);
    allFunctionsSet.unite(userFunctionsSet);
    allSymbolsSet.unite(userFunctionsSet);

    QString prefix = m_pendingView->document()->text(m_pendingRange);

    beginResetModel();
    m_matches.clear();

    for (const QString& match : allSymbolsSet)
    {
        // For member access (empty prefix), we match everything.
        // For normal typing, we match based on the prefix.
        if (prefix.isEmpty() || match.startsWith(prefix, Qt::CaseInsensitive))
        {
            m_matches.append({match, allFunctionsSet.contains(match), allKeywordsSet.contains(match)});
        }
    }

    std::sort(m_matches.begin(), m_matches.end(), [](const auto& a, const auto& b)
    {
        return a.name.localeAwareCompare(b.name) < 0;
    });

    setRowCount(m_matches.size());
    endResetModel();

    Q_EMIT modelIsReady(m_matches);
}

QVariant CantorCompletionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_matches.count())
        return QVariant();

    // Only DisplayRole is used for autocompletion
    if (role == Qt::DisplayRole) {
        return m_matches.at(index.row()).name;
    }

    return QVariant();
}

void CantorCompletionModel::executeCompletionItem(KTextEditor::View* view, const KTextEditor::Range &, const QModelIndex &index) const
{
    if (!index.isValid() || !view) return;

    const CompletionItem& item = m_matches.at(index.row());
    QString textToInsert = item.name;

    // Insert parentheses and move the cursor only if this is a function
    if (item.isFunction)
    {
        textToInsert += QStringLiteral("()");
    }
    else if (item.isKeyword)
    {
        textToInsert += QLatin1Char(' ');
    }

    KTextEditor::Range rangeToReplace = view->document()->wordRangeAt(view->cursorPosition());
    view->document()->replaceText(rangeToReplace, textToInsert);

    // Move the cursor inside the parentheses for user convenience
    if (item.isFunction)
    {
        KTextEditor::Cursor newCursorPos = rangeToReplace.start();
        newCursorPos.setColumn(newCursorPos.column() + item.name.length() + 1);
        view->setCursorPosition(newCursorPos);
    }
}

KTextEditor::Range CantorCompletionModel::completionRange(KTextEditor::View* view, const KTextEditor::Cursor &cursor)
{
    return view->document()->wordRangeAt(cursor);
}

bool CantorCompletionModel::shouldStartCompletion(KTextEditor::View*, const QString &insertedText, bool, const KTextEditor::Cursor &)
{
    // Start autocompletion if the user entered a letter or "_"
    if (!insertedText.isEmpty())
    {
        const QChar lastChar = insertedText.back();
        if (lastChar.isLetter() || lastChar == QLatin1Char('_') || lastChar == QLatin1Char('.'))
        {
            return true;
        }
    }
    return false;
}
