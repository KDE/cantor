#include "cantorcompletionmodel.h"
#include "worksheettexteditoritem.h"
#include "worksheet.h"
#include "lib/session.h"
#include "lib/keywordsmanager.h"
#include "lib/defaultvariablemodel.h"
#include "lib/textresult.h"

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
        m_session = parent->worksheet()->session();

    m_debounceTimer = new QTimer(this);
    m_debounceTimer->setSingleShot(true);
    m_debounceTimer->setInterval(200);
    connect(m_debounceTimer, &QTimer::timeout, this, &CantorCompletionModel::startCompletionRequest);
}

CantorCompletionModel::~CantorCompletionModel()
{
    m_debounceTimer->stop();
}

void CantorCompletionModel::completionInvoked(KTextEditor::View* view, const KTextEditor::Range& range, InvocationType invocationType)
{
    m_completionContextObject.clear();
    if (range.start().column() > 0)
    {
        KTextEditor::Cursor precedingCursor(range.start().line(),
                                            range.start().column() - 1);
        const QChar precedingChar = view->document()->characterAt(precedingCursor);

        if (precedingChar == QLatin1Char('.'))
        {
            const auto& objectRange = view->document()->wordRangeAt(precedingCursor);
            if (objectRange.isValid())
                m_completionContextObject = view->document()->text(objectRange);
        }
    }

    m_pendingView = view;
    m_pendingRange = range;
    m_debounceTimer->start();
}

void CantorCompletionModel::startCompletionRequest()
{
    if (!m_pendingView || !m_session)
        return;

    KTextEditor::Cursor currentPos = m_pendingView->cursorPosition();
    if (currentPos.line() != m_pendingRange.start().line())
        return;

    if (currentPos.column() > 0)
    {
        QChar lastChar = m_pendingView->document()->characterAt(KTextEditor::Cursor(currentPos.line(), currentPos.column() - 1));
        if (!lastChar.isLetterOrNumber() && lastChar != QLatin1Char('_') && lastChar != QLatin1Char('.'))
            return;
    }
    else
        return;

    // Member completion branch
    if (!m_completionContextObject.isEmpty())
    {
        const QString command = QStringLiteral(R"python(
            __cantor_results__ = []
            try:
                for __cantor_member__ in dir(%1):
                    __cantor_attr__ = getattr(%1, __cantor_member__)
                    __cantor_is_callable__ = '1' if callable(__cantor_attr__) else '0'
                    __cantor_results__.append(f"{__cantor_member__},{__cantor_is_callable__}")
                print(';'.join(__cantor_results__))
            except:
                pass
        )python").arg(m_completionContextObject);

        auto* expr = m_session->evaluateExpression(command,
                                                   Cantor::Expression::DeleteOnFinish,
                                                   true);
        if (expr)
            connect(expr, &Cantor::Expression::statusChanged, this, &CantorCompletionModel::handleMemberCompletionResult);
        m_completionContextObject.clear();
        return;
    }

    const auto* keywordsManager = m_session->keywordsManager();
    const auto* varModel = m_session->variableModel();
    if (!keywordsManager || !varModel)
        return;

    QSet<QString> allSymbolsSet;
    QSet<QString> allFunctionsSet;
    QSet<QString> allKeywordsSet;

    const QStringList staticLists = keywordsManager->symbolLists();
    for (const QString& listName : staticLists)
    {
        const QSet<QString>& symbols = keywordsManager->symbolList(listName);
        allSymbolsSet.unite(symbols);

        if (listName.contains(QStringLiteral("func"), Qt::CaseInsensitive))
            allFunctionsSet.unite(symbols);

        if (listName == QStringLiteral("import") || listName == QStringLiteral("flow") ||
            listName == QStringLiteral("flow_yield") || listName == QStringLiteral("defs") ||
            listName == QStringLiteral("exceptions") || listName == QStringLiteral("patternmatching"))
            allKeywordsSet.unite(symbols);
    }

    const QStringList userVariables = varModel->variableNames();
    const QStringList userFunctions = varModel->functions();

    const QSet<QString> userVariablesSet(userVariables.begin(), userVariables.end());
    const QSet<QString> userFunctionsSet(userFunctions.begin(), userFunctions.end());

    allSymbolsSet.unite(userVariablesSet);
    allFunctionsSet.unite(userFunctionsSet);

    KTextEditor::Range currentWordRange = m_pendingView->document()->wordRangeAt(currentPos);
    QString prefix = m_pendingView->document()->text(currentWordRange);

    if (prefix.trimmed().isEmpty())
        return;

    beginResetModel();
    m_matches.clear();

    for (const QString& match : allSymbolsSet)
    {
        if (prefix.isEmpty() || match.startsWith(prefix, Qt::CaseInsensitive))
        {
            bool isFunc = allFunctionsSet.contains(match);
            bool isKey = allKeywordsSet.contains(match);
            bool isVar = userVariablesSet.contains(match);

            m_matches.append({match, isFunc, isKey, false, isVar});
        }
    }

    std::sort(m_matches.begin(), m_matches.end(), [](const auto& a, const auto& b) {
        return a.name.localeAwareCompare(b.name) < 0;
    });

    setRowCount(0);
    endResetModel();

    Q_EMIT modelIsReady(m_matches);
}

void CantorCompletionModel::handleMemberCompletionResult(Cantor::Expression::Status status)
{
    if (status != Cantor::Expression::Done)
        return;

    if (m_pendingView && m_pendingView->cursorPosition().line() != m_pendingRange.start().line())
        return;

    if (m_pendingView)
    {
        KTextEditor::Cursor currentPos = m_pendingView->cursorPosition();
        if (currentPos.column() > 0)
        {
            QChar lastChar = m_pendingView->document()->characterAt(KTextEditor::Cursor(currentPos.line(), currentPos.column() - 1));
            if (!lastChar.isLetterOrNumber() && lastChar != QLatin1Char('_') && lastChar != QLatin1Char('.'))
                return;
        }
    }

    auto* expr = qobject_cast<Cantor::Expression*>(sender());
    if (!expr || expr->results().isEmpty())
        return;

    auto* result = expr->result();
    if (!result || result->type() != Cantor::TextResult::Type)
        return;

    const QString resultString = result->data().toString().trimmed();
    const QStringList records = resultString.split(QStringLiteral(";"), Qt::SkipEmptyParts);

    beginResetModel();
    m_matches.clear();

    KTextEditor::Range currentWordRange = m_pendingView->document()->wordRangeAt(m_pendingView->cursorPosition());
    const QString prefix = m_pendingView->document()->text(currentWordRange);

    for (const QString& record : records)
    {
        const QStringList parts = record.split(QStringLiteral(","));
        if (parts.size() != 2) continue;
        const QString name = parts[0];
        const bool isFunction = (parts[1] == QLatin1String("1"));

        if (!name.startsWith(QLatin1String("__")) && (prefix.isEmpty() || name.startsWith(prefix, Qt::CaseInsensitive)))
            m_matches.append({name, isFunction, false, !isFunction, false});
    }

    std::sort(m_matches.begin(), m_matches.end(), [](const auto& a, const auto& b) {
        return a.name.localeAwareCompare(b.name) < 0;
    });

    setRowCount(0);
    endResetModel();

    Q_EMIT modelIsReady(m_matches);
}

QVariant CantorCompletionModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_matches.count())
        return QVariant();

    if (role == Qt::DisplayRole)
        return m_matches.at(index.row()).name;

    return QVariant();
}

void CantorCompletionModel::executeCompletionItem(KTextEditor::View* view, const KTextEditor::Range&, const QModelIndex& index) const
{
    if (!index.isValid() || !view)
        return;

    const CompletionItem& item = m_matches.at(index.row());
    QString textToInsert = item.name;

    if (item.isFunction)
        textToInsert += QStringLiteral("()");
    else if (item.isKeyword || item.isAttribute || item.isVariable)
        textToInsert += QLatin1Char(' ');

    KTextEditor::Range rangeToReplace = view->document()->wordRangeAt(view->cursorPosition());
    view->document()->replaceText(rangeToReplace, textToInsert);

    if (item.isFunction)
    {
        KTextEditor::Cursor newCursorPos = rangeToReplace.start();
        newCursorPos.setColumn(newCursorPos.column() + item.name.length() + 1);
        view->setCursorPosition(newCursorPos);
    }
}

void CantorCompletionModel::executeCompletionItem(KTextEditor::View* view, int row)
{
    if (!view || row < 0 || row >= m_matches.count())
        return;

    const CompletionItem& item = m_matches.at(row);
    QString textToInsert = item.name;

    if (item.isFunction)
        textToInsert += QStringLiteral("()");
    else if (item.isKeyword || item.isAttribute || item.isVariable)
        textToInsert += QLatin1Char(' ');

    KTextEditor::Range rangeToReplace = view->document()->wordRangeAt(view->cursorPosition());

    view->document()->replaceText(rangeToReplace, textToInsert);

    if (item.isFunction)
    {
        KTextEditor::Cursor newCursorPos = rangeToReplace.start();
        newCursorPos.setColumn(newCursorPos.column() + item.name.length() + 1);
        view->setCursorPosition(newCursorPos);
    }
}

KTextEditor::Range CantorCompletionModel::completionRange(KTextEditor::View* view, const KTextEditor::Cursor& cursor)
{
    return view->document()->wordRangeAt(cursor);
}

bool CantorCompletionModel::shouldStartCompletion(KTextEditor::View*, const QString& insertedText, bool, const KTextEditor::Cursor&)
{
    if (!insertedText.isEmpty())
    {
        const QChar lastChar = insertedText.back();
        if (lastChar.isLetter() || lastChar == QLatin1Char('_') || lastChar == QLatin1Char('.'))
            return true;
    }
    return false;
}

void CantorCompletionModel::abortCompletion()
{
    if (m_debounceTimer && m_debounceTimer->isActive())
        m_debounceTimer->stop();
}
