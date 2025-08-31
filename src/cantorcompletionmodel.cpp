#include "cantorcompletionmodel.h"
#include "worksheettexteditoritem.h"
#include "worksheet.h"
#include "lib/session.h"
#include "lib/symbolmanager.h"
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
    {
        m_session = parent->worksheet()->session();
    }

    m_debounceTimer = new QTimer(this);
    m_debounceTimer->setSingleShot(true);
    m_debounceTimer->setInterval(200);
    connect(m_debounceTimer, &QTimer::timeout,
            this, &CantorCompletionModel::startCompletionRequest);
}

CantorCompletionModel::~CantorCompletionModel()
{
    m_debounceTimer->stop();
}

void CantorCompletionModel::completionInvoked(KTextEditor::View* view,
                                              const KTextEditor::Range& range,
                                              InvocationType invocationType)
{
    m_completionContextObject.clear();
    if (range.start().column() > 0)
    {
        KTextEditor::Cursor precedingCursor(range.start().line(),
                                            range.start().column() - 1);
        const QChar precedingChar = view->document()->characterAt(precedingCursor);

        if (precedingChar == QLatin1Char('.'))
        {
            const KTextEditor::Range objectRange =
                view->document()->wordRangeAt(precedingCursor);
            if (objectRange.isValid())
            {
                m_completionContextObject =
                    view->document()->text(objectRange);
            }
        }
    }

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
        {
            connect(expr, &Cantor::Expression::statusChanged,
                    this, &CantorCompletionModel::handleMemberCompletionResult);
        }
        m_completionContextObject.clear();
        return;
    }

    // Global completion branch
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
        if (prefix.isEmpty() || match.startsWith(prefix, Qt::CaseInsensitive))
        {
            m_matches.append({match,
                              allFunctionsSet.contains(match),
                              allKeywordsSet.contains(match),
                              false});
        }
    }

    std::sort(m_matches.begin(), m_matches.end(),
              [](const auto& a, const auto& b)
              {
                  return a.name.localeAwareCompare(b.name) < 0;
              });

    setRowCount(m_matches.size());
    endResetModel();

    Q_EMIT modelIsReady(m_matches);
}

void CantorCompletionModel::handleMemberCompletionResult(Cantor::Expression::Status status)
{
    if (status != Cantor::Expression::Done)
    {
        return;
    }

    auto* expr = qobject_cast<Cantor::Expression*>(sender());
    if (!expr || expr->results().isEmpty())
    {
        return;
    }

    auto* result = expr->result();
    if (!result || result->type() != Cantor::TextResult::Type)
    {
        return;
    }

    const QString resultString = result->data().toString().trimmed();
    const QStringList records = resultString.split(QStringLiteral(";"),
                                                   Qt::SkipEmptyParts);

    beginResetModel();
    m_matches.clear();

    const QString prefix = m_pendingView->document()->text(m_pendingRange);

    for (const QString& record : records)
    {
        const QStringList parts = record.split(QStringLiteral(","));
        if (parts.size() != 2)
        {
            continue;
        }

        const QString name = parts[0];
        const bool isFunction = (parts[1] == QLatin1String("1"));

        if (!name.startsWith(QLatin1String("__"))
            && (prefix.isEmpty() || name.startsWith(prefix, Qt::CaseInsensitive)))
        {
            // Member can be either a function (method) or an attribute
            m_matches.append({name, isFunction, false, !isFunction});
        }
    }

    std::sort(m_matches.begin(), m_matches.end(),
              [](const auto& a, const auto& b)
              {
                  return a.name.localeAwareCompare(b.name) < 0;
              });

    setRowCount(m_matches.size());
    endResetModel();

    Q_EMIT modelIsReady(m_matches);
}

QVariant CantorCompletionModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_matches.count())
    {
        return QVariant();
    }

    if (role == Qt::DisplayRole)
    {
        return m_matches.at(index.row()).name;
    }

    return QVariant();
}

void CantorCompletionModel::executeCompletionItem(KTextEditor::View* view, const KTextEditor::Range&, const QModelIndex& index) const
{
    if (!index.isValid() || !view)
    {
        return;
    }

    const CompletionItem& item = m_matches.at(index.row());
    QString textToInsert = item.name;

    if (item.isFunction)
    {
        textToInsert += QStringLiteral("()");
    }
    else if (item.isKeyword || item.isAttribute)
    {
        textToInsert += QLatin1Char(' ');
    }

    KTextEditor::Range rangeToReplace =
        view->document()->wordRangeAt(view->cursorPosition());
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
        if (lastChar.isLetter()
            || lastChar == QLatin1Char('_')
            || lastChar == QLatin1Char('.'))
        {
            return true;
        }
    }
    return false;
}
