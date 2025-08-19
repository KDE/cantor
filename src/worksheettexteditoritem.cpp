#include "worksheettexteditoritem.h"
#include "worksheet.h"
#include "worksheetentry.h"
#include "lib/renderer.h"
#include "lib/session.h"
#include "worksheetcursor.h"
#include "worksheetview.h"

#include <QApplication>
#include <QClipboard>
#include <KColorScheme>
#include <QAbstractTextDocumentLayout>
#include <QTextBlock>
#include <QTextLine>
#include <QTimer>
#include <QGraphicsSceneResizeEvent>
#include <QPainter>
#include <QScrollBar>

#include <QAction>
#include <QMimeData>
#include <QColorDialog>
#include <KColorScheme>
#include <QFontDatabase>

WorksheetTextEditorItem::WorksheetTextEditorItem(EditorMode initialMode, WorksheetEntry *parentEntry, QGraphicsItem *parent)
    : QGraphicsProxyWidget(parent),
    m_mode(initialMode),
    m_dblClickBehaviour(ImageReplacement),
    m_completionEnabled(true),
    m_completionActive(false),
    m_dragEnabled(false),
    m_parentEntry(parentEntry),
    m_customCompleter(nullptr)
{
    m_editor = KTextEditor::Editor::instance();
    m_document = m_editor->createDocument(nullptr);
    m_view = m_document->createView(nullptr);

    m_view->setConfigValue(QStringLiteral("scrollbar-minimap"), false);
    m_view->setConfigValue(QStringLiteral("scrollbar-preview"), false);
    m_view->setConfigValue(QStringLiteral("folding-bar"), false);
    m_view->setConfigValue(QStringLiteral("folding-preview"), false);
    m_view->setConfigValue(QStringLiteral("line-numbers"), false);

    if (m_view)
    {
        if (QScrollBar* vScrollBar = m_view->verticalScrollBar())
        {
            vScrollBar->setFixedWidth(0);
        }
        if (QScrollBar* hScrollBar = m_view->horizontalScrollBar())
        {
            hScrollBar->setFixedHeight(0);
        }
    }
    setWidget(m_view);

    const auto theme = m_view->theme();

    if (theme.isValid())
    {
        m_themeDefaultBackgroundColor = theme.editorColor(KSyntaxHighlighting::Theme::BackgroundColor);
    }
    else
    {
        m_themeDefaultBackgroundColor = m_view->palette().color(QPalette::Base);
    }

    setFocusPolicy(Qt::StrongFocus);
    m_view->setFocusPolicy(Qt::StrongFocus);
    m_view->setFocus();

    QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    m_view->setFont(fixedFont);
    m_currentFont = fixedFont;
    m_currentFontPointSize = m_currentFont.pointSize() > 0 ? m_currentFont.pointSize() : 14;
    m_view->setStatusBarEnabled(false);
    setAcceptDrops(true);

    m_view->setAnnotationBorderVisible(false);
    m_view->setAttribute(Qt::WA_TranslucentBackground, false);

    setupLineHeight();

    bool isEditable = (m_mode == EditorMode::Editable);
    m_document->setReadWrite(isEditable);
    if(isEditable)
    {
        m_completionModel = new CantorCompletionModel(this);
        m_view->registerCompletionModel(m_completionModel);
        connect(m_completionModel, &CantorCompletionModel::modelIsReady, this, &WorksheetTextEditorItem::showCustomCompleter);

        auto* model = session()->variableModel();
        if (model) {
            connect(model, &Cantor::DefaultVariableModel::initialModelPopulated, this, [this]() {
                KTextEditor::Range range = m_view->document()->wordRangeAt(m_view->cursorPosition());
                m_completionModel->completionInvoked(m_view, range, KTextEditor::CodeCompletionModel::AutomaticInvocation);
            });
        }
        connect(this, &WorksheetTextEditorItem::sizeChanged, parentEntry, &WorksheetEntry::recalculateSize);
    }

    QRectF viewRect = m_view->contentsRect();
    m_size = QSizeF(viewRect.width(), viewRect.height());

    connect(m_view, &KTextEditor::View::cursorPositionChanged, this, [this]()
    {
        auto cursor = m_view->cursorPosition();
        if (!cursor.isValid()) {
            return;
        }
        int lineLength = m_document->lineLength(cursor.line());
        QString textLine = m_document->text(KTextEditor::Range(cursor.line(), 0, cursor.line(), lineLength));

        static QRegularExpression urlRegex(QStringLiteral("https?://[^\\s]+"));
        auto match = urlRegex.match(textLine);
        if(match.hasMatch())
            QApplication::setOverrideCursor(Qt::PointingHandCursor);
        else
            QApplication::restoreOverrideCursor();
    });

    connect(m_document, &KTextEditor::Document::textChanged, this, &WorksheetTextEditorItem::testSize);

    connect(this, &WorksheetTextEditorItem::menuCreated, parentEntry, &WorksheetEntry::populateMenu, Qt::DirectConnection);
    connect(this, &WorksheetTextEditorItem::deleteEntry, [parentEntry](){ parentEntry->startRemoving(); });

    connect(m_document, &KTextEditor::Document::textChanged, this, [this]()
    {
        bool canUndo = m_document->isModified();
        Q_EMIT undoAvailable(canUndo);
        Q_EMIT redoAvailable(canUndo);
        Q_EMIT copyAvailable(canUndo);
        Q_EMIT cutAvailable(canUndo);
        Q_EMIT pasteAvailable(canUndo);
    });

    connect(this, &WorksheetTextEditorItem::receivedFocus, this, [this]()
    {
        if(!m_view->hasFocus())
        {
            m_view->setFocus();
        }
    });

    m_view->installEventFilter(this);
}

WorksheetTextEditorItem::~WorksheetTextEditorItem()
{
    if(worksheet() && this == worksheet()->LastFocusedTextItem())
        worksheet()->updateFocusedTextItem(static_cast<WorksheetTextEditorItem*>(nullptr));
    if(worksheet())
        worksheet()->removeRequestedWidth(this);
    if (m_customCompleter) {
        delete m_customCompleter;
    }
}

KTextEditor::View* WorksheetTextEditorItem::view() const
{
    return m_view;
}

KTextEditor::Document* WorksheetTextEditorItem::document() const
{
    return m_document;
}

QString WorksheetTextEditorItem::toPlainText() const
{
    return m_document? m_document->text() : QString();
}

void WorksheetTextEditorItem::setPlainText(const QString &text)
{
    m_document->setText(text);
}

void WorksheetTextEditorItem::clear()
{
    m_document->clear();
}

void WorksheetTextEditorItem::insertText(const QString &text)
{
    m_document->insertText(m_view->cursorPosition(), text);
}

void WorksheetTextEditorItem::clearSelection()
{
    m_view->setSelection(KTextEditor::Range::invalid());
    selectionChanged();
}

void WorksheetTextEditorItem::setBackgroundColor(const QColor &color)
{
    QColor bgColorToApply = color.isValid() ? color : m_themeDefaultBackgroundColor;
    QPalette pal = m_view->palette();
    pal.setColor(QPalette::Base, bgColorToApply);
    m_view->setPalette(pal);

    m_view->setConfigValue(QStringLiteral("background-color"), bgColorToApply.name());
}

QColor WorksheetTextEditorItem::backgroundColor() const
{
    if (m_view)
        return m_view->palette().color(QPalette::Base);
    return QColor();
}


void WorksheetTextEditorItem::setDefaultTextColor(const QColor &color)
{
    if (m_defaultTextColorRange)
    {
        delete m_defaultTextColorRange;
        m_defaultTextColorRange = nullptr;
    }

    m_defaultTextColor = color;

    if (color.isValid())
    {
        m_defaultTextColorAttribute = KTextEditor::Attribute::Ptr(new KTextEditor::Attribute());
        m_defaultTextColorAttribute->setForeground(color);

        m_defaultTextColorRange = m_document->newMovingRange(m_document->documentRange());
        m_defaultTextColorRange->setAttribute(m_defaultTextColorAttribute);
    }
    else
    {
        m_defaultTextColorAttribute->clear();
    }

    m_view->update();
}


QColor WorksheetTextEditorItem::defaultTextColor() const
{
    if (m_defaultTextColor.isValid())
    {
        return m_defaultTextColor;
    }
    return m_themeDefaultTextColor;
}

void WorksheetTextEditorItem::setupLineHeight()
{
    if (!m_view || !m_document)
    {
        return;
    }

    QFont font = m_view->font();
    if (font.pointSize() < 10)
    {
        font.setPointSize(14);
        m_currentFontPointSize = 14;
        m_view->setFont(font);
        m_currentFont = font;
    }
    else
    {
        m_currentFontPointSize = font.pointSize();
        m_currentFont = font;
    }
    m_view->setConfigValue(QStringLiteral("font"), m_currentFont);
    QFontMetrics metrics(m_view->font());
    int minHeight = metrics.height();
    m_view->setMinimumHeight(minHeight);
}

QRectF WorksheetTextEditorItem::cursorRect() const
{
    if (!m_view)
    {
        return QRectF();
    }

    const int lineHeight = m_view->fontMetrics().height();
    const KTextEditor::Range selection = m_view->selectionRange();

    if (!selection.isValid() || selection.isEmpty())
    {
        const QPoint cursorPoint = m_view->cursorToCoordinate(m_view->cursorPosition());
        return QRectF(cursorPoint.x(), cursorPoint.y(), 1, lineHeight);
    }

    const QPoint startPoint = m_view->cursorToCoordinate(selection.start());
    const QPoint endPoint = m_view->cursorToCoordinate(selection.end());

    if (selection.start().line() == selection.end().line())
    {
        return QRectF(startPoint, endPoint).normalized().adjusted(0, 0, 0, lineHeight);
    }

    qreal selectionTop = qMin(startPoint.y(), endPoint.y());
    qreal selectionBottom = qMax(startPoint.y(), endPoint.y()) + lineHeight;
    return QRectF(0, selectionTop, m_view->width(), selectionBottom - selectionTop);
}

QRectF WorksheetTextEditorItem::sceneCursorRect() const
{
    return mapRectToScene(cursorRect());
}

void WorksheetTextEditorItem::setFocusAt(int pos, qreal x)
{
    KTextEditor::Cursor cursor = m_view->cursorPosition();

    if (pos == TopLeft)
    {
        cursor.setPosition(0, 0);
    }
    else if (pos == BottomRight)
    {
        int lastLine = m_document->lines() - 1;
        int lastColumn = m_document->line(lastLine).length();
        cursor.setPosition(lastLine, lastColumn);
    }
    else
    {
        int targetLine = (pos == TopCoord) ? 0 : (m_document->lines() - 1);
        QString lineText = m_document->line(targetLine);

        int targetColumn = 0;
        QFontMetrics metrics(m_view->font());
        int widthAccumulated = 0;
        for (int col = 0; col < lineText.length(); ++col)
        {
            int charWidth = metrics.horizontalAdvance(lineText[col]);
            if (widthAccumulated + charWidth / 2 >= x)
            {
                targetColumn = col;
                break;
            }
            widthAccumulated += charWidth;
        }
        if (widthAccumulated < x)
        {
            targetColumn = lineText.length();
        }

        cursor.setPosition(targetLine, targetColumn);
    }

    m_view->setCursorPosition(cursor);
    Q_EMIT cursorPositionChanged(cursor);
    m_view->setFocus();
}


bool WorksheetTextEditorItem::isEditable() const
{
    return m_mode == Editable;
}

void WorksheetTextEditorItem::setReadOnly(bool readOnly)
{
    m_document->setReadWrite(!readOnly);
}

bool WorksheetTextEditorItem::isReadOnly() const
{
    return m_document->isReadWrite();
}

void WorksheetTextEditorItem::setDoubleClickBehaviour(DoubleClickEventBehaviour behaviour)
{
    m_dblClickBehaviour = behaviour;
}

bool WorksheetTextEditorItem::completionEnabled() const
{
    return m_completionEnabled;
}

void WorksheetTextEditorItem::enableCompletion(bool enable)
{
    m_completionEnabled = enable;
}

bool WorksheetTextEditorItem::completionActive() const
{
    return m_completionActive;
}

void WorksheetTextEditorItem::activateCompletion(bool activate)
{
    m_completionActive = activate;
}

void WorksheetTextEditorItem::setSyntaxHighlightingMode(const QString& modeName)
{
    if(m_document)
    {
        m_document->setHighlightingMode(modeName);
    }
}

QString WorksheetTextEditorItem::syntaxHighlightingMode() const
{
    if(m_document)
    {
        return m_document->highlightingMode();
    }
    return QString();
}

void WorksheetTextEditorItem::setDragEnabled(bool enabled)
{
    m_dragEnabled = enabled;
}

bool WorksheetTextEditorItem::isDragEnabled() const
{
    return m_dragEnabled;
}

void WorksheetTextEditorItem::populateMenu(QMenu *menu, QPointF globalPos)
{
    auto* cut = KStandardAction::cut(this, &WorksheetTextEditorItem::cut, menu);
    auto* copy = KStandardAction::copy(this, &WorksheetTextEditorItem::copy, menu);
    auto* paste = KStandardAction::paste(this, &WorksheetTextEditorItem::paste, menu);
    KTextEditor::View* view = m_view;
    bool hasSelection = m_view->selectionRange().isValid();
    if(!hasSelection)
    {
        cut->setEnabled(false);
        copy->setEnabled(false);
    }
    if(QApplication::clipboard()->text().isEmpty())
    {
        paste->setEnabled(false);
    }
    bool actionAdded = false;
    if(isEditable())
    {
        menu->addAction(cut);
        actionAdded = true;
    }
    if(!m_dragEnabled && flags() & Qt::TextSelectableByMouse)
    {
        menu->addAction(copy);
        actionAdded = true;
    }
    if(isEditable())
    {
        menu->addAction(paste);
        actionAdded = true;
    }
    if(actionAdded)
        menu->addSeparator();

    Q_EMIT menuCreated(menu, mapToParent(globalPos));
}

double WorksheetTextEditorItem::width() const
{
    return m_view->width();
}

double WorksheetTextEditorItem::height() const
{
    return m_view ? m_view->height() : 0;
}

QSizeF WorksheetTextEditorItem::estimateContentSize(qreal maxWidth) const
{
    if (!m_document || !m_view)
        return QSizeF();

    const QFontMetricsF fm(m_view->font());
    qreal maxLineWidth = 0;
    qreal totalHeight = 0;

    const int lineCount = qMax(1, m_document->lines());
    for (int i = 0; i < lineCount; ++i)
    {
        QString textLine = m_document->line(i);
        QTextLayout layout(textLine, m_view->font());
        layout.beginLayout();
        while (true)
        {
            QTextLine line = layout.createLine();
            if (!line.isValid())
                break;
            if (maxWidth > 0)
                line.setLineWidth(maxWidth);
            line.setPosition(QPointF(0, totalHeight));
            totalHeight += line.height();
            maxLineWidth = qMax<qreal>(maxLineWidth, line.naturalTextWidth());
        }
        layout.endLayout();
    }

    if (maxWidth > 0)
        maxLineWidth = qMin<qreal>(maxLineWidth, maxWidth);

    return QSizeF(qMax(maxLineWidth, 1.0), qMax(totalHeight, fm.lineSpacing()));
}

qreal WorksheetTextEditorItem::setGeometry(qreal x, qreal y, qreal w, bool centered)
{
    prepareGeometryChange();

    QSizeF contentSize = estimateContentSize(w);

    if(contentSize.width() < w && centered)
        setPos(x + w/2 - contentSize.width()/2, y);
    else
        setPos(x, y);

    m_size = contentSize;

    resize(w, contentSize.height());
    worksheet()->setRequestedWidth(this, scenePos().x() + contentSize.width() - 10);

    return contentSize.height();
}

Worksheet* WorksheetTextEditorItem::worksheet()
{
    return qobject_cast<Worksheet*>(scene());
}

WorksheetView * WorksheetTextEditorItem::worksheetView()
{
    return worksheet()->worksheetView();
}

WorksheetEntry* WorksheetTextEditorItem::worksheetEntry()
{
    return m_parentEntry;
}

bool WorksheetTextEditorItem::isUndoAvailable() const
{
    return m_document && m_document->isModified();
}

bool WorksheetTextEditorItem::isRedoAvailable() const
{
    return false;
}

bool WorksheetTextEditorItem::isCutAvailable() const
{
    return m_view && m_view->selection();
}

bool WorksheetTextEditorItem::isCopyAvailable() const
{
    return !m_dragEnabled && m_view && m_view->selection();
}

bool WorksheetTextEditorItem::isPasteAvailable() const
{
    return isEditable() && !QApplication::clipboard()->text().isEmpty();
}

KTextEditor::Range WorksheetTextEditorItem::search(const QString &pattern, KTextEditor::SearchOptions options, const KTextEditor::Cursor &start)
{
    if (!m_document)
    {
        return KTextEditor::Range::invalid();
    }

    KTextEditor::Range searchRange;
    if (start.isValid())
    {
        if (options & KTextEditor::Backwards)
        {
            searchRange = KTextEditor::Range(KTextEditor::Cursor(0, 0), start);
        }
        else
        {
            searchRange = KTextEditor::Range(start, m_document->documentEnd());
        }
    }
    else
    {
        searchRange = KTextEditor::Range::invalid();
    }

    QList<KTextEditor::Range> matches = m_document->searchText(searchRange, pattern, options);

    if (matches.isEmpty()) {
        return KTextEditor::Range::invalid();
    }

    return (options & KTextEditor::Backwards) ? matches.last() : matches.first();
}

bool WorksheetTextEditorItem::replace(const QString &replacement)
{
    if (!isEditable() || !m_view->selection())
    {
        return false;
    }

    KTextEditor::Range selection = m_view->selectionRange();
    if (selection.isValid() && !selection.isEmpty())
    {
        m_document->replaceText(selection, replacement);
        m_view->setSelection(KTextEditor::Range::invalid());
        return true;
    }
    return false;
}

void WorksheetTextEditorItem::replaceAll(const QString &pattern, const QString &replacement, KTextEditor::SearchOptions options)
{
    if (!isEditable() || pattern.isEmpty())
    {
        return;
    }
    options &= ~KTextEditor::SearchOption::Backwards;

    KTextEditor::Cursor searchStartCursor(0, 0);
    while (true) {
        KTextEditor::Range foundRange = search(pattern, options, searchStartCursor);
        if (!foundRange.isValid())
        {
            break;
        }
        m_document->replaceText(foundRange, replacement);
        searchStartCursor.setPosition(foundRange.start().line(), foundRange.start().column() + replacement.length());
    }
}

void WorksheetTextEditorItem::setTextBackgroundColor()
{
    if (!m_view || !m_document)
        return;
    QColor current = backgroundColor();
    QColor chosen = QColorDialog::getColor(current, worksheetView());
    if (!chosen.isValid())
        return;
    setBackgroundColor(chosen);
}

void WorksheetTextEditorItem::setTextBold(bool bold)
{
    if (!m_view)
        return;
    m_currentFont.setBold(bold);
    applyFontState();
}

void WorksheetTextEditorItem::setTextItalic(bool italic)
{
    if (!m_view)
        return;
    m_currentFont.setItalic(italic);
    applyFontState();
}

void WorksheetTextEditorItem::setTextUnderline(bool underline)
{
    if (!m_view)
        return;
    m_currentFont.setUnderline(underline);
    applyFontState();
}

void WorksheetTextEditorItem::setTextStrikeOut(bool strikeOut)
{
    if (!m_view)
        return;
    m_currentFont.setStrikeOut(strikeOut);
    applyFontState();
}

void WorksheetTextEditorItem::setAlignment(Qt::Alignment alignment)
{
    Q_UNUSED(alignment);
}

void WorksheetTextEditorItem::setFontFamily(const QString &family)
{
    if (!m_view)
        return;
    if (!family.isEmpty())
        m_currentFont.setFamily(family);
    applyFontState();
}

void WorksheetTextEditorItem::setFontSize(int size)
{
    if (!m_view || size <= 0)
        return;
    m_currentFontPointSize = size;
    m_currentFont.setPointSize(m_currentFontPointSize);
    applyFontState();
}

void WorksheetTextEditorItem::applyFontState()
{
    m_currentFont.setPointSize(m_currentFontPointSize);
    m_view->setFont(m_currentFont);
    m_view->setConfigValue(QStringLiteral("font"), m_currentFont);

    testSize();
}

void WorksheetTextEditorItem::setFont(const QFont &font)
{
    if (!m_view)
        return;

    if (font.pointSize() > 0)
    {
        m_currentFontPointSize = font.pointSize();
    }

    m_currentFont = font;
    m_currentFont.setPointSize(m_currentFontPointSize);
    applyFontState();
}

void WorksheetTextEditorItem::setTheme(const QString& themeName)
{
    if (!m_view) return;

    m_view->setConfigValue(QStringLiteral("theme"), themeName);


    const auto& theme = m_view->theme();
    if (theme.isValid())
    {
        m_themeDefaultBackgroundColor = theme.editorColor(KSyntaxHighlighting::Theme::BackgroundColor);
    }

    setBackgroundColor(QColor());
}

void WorksheetTextEditorItem::increaseFontSize()
{
    if (!m_view)
    {
        return;
    }

    QFontDatabase fdb;
    const QList<int> sizes = fdb.pointSizes(m_currentFont.family());
    if (sizes.isEmpty())
    {
        setFontSize(m_currentFontPointSize + 1);
        return;
    }

    for (int i = 0; i < sizes.size(); ++i)
    {
        if (m_currentFontPointSize >= sizes.at(i) && (i + 1 < sizes.size()))
        {
            if (m_currentFontPointSize < sizes.at(i+1)) {
                setFontSize(sizes.at(i + 1));
                return;
            }
        }
    }
    setFontSize(m_currentFontPointSize + 1);
}

void WorksheetTextEditorItem::decreaseFontSize()
{
    if (!m_view)
    {
        return;
    }

    QFontDatabase fdb;
    const QList<int> sizes = fdb.pointSizes(m_currentFont.family());
    if (sizes.isEmpty())
    {
        if (m_currentFontPointSize > 1)
        {
            setFontSize(m_currentFontPointSize - 1);
        }
        return;
    }

    for (int i = sizes.size() - 1; i >= 0; --i)
    {
        if (m_currentFontPointSize <= sizes.at(i) && (i - 1 >= 0))
        {
            if (m_currentFontPointSize > sizes.at(i-1))
            {
                setFontSize(sizes.at(i - 1));
                return;
            }
        }
    }

    if (m_currentFontPointSize > 1)
    {
        setFontSize(m_currentFontPointSize - 1);
    }
}

void WorksheetTextEditorItem::undo()
{
    if (m_document)
    {
        QMetaObject::invokeMethod(m_document, "undo");
    }
}

void WorksheetTextEditorItem::redo()
{
    if (m_document)
    {
        QMetaObject::invokeMethod(m_document, "redo");
    }
}

void WorksheetTextEditorItem::cut()
{
    copy();
    KTextEditor::Range rangeText = m_view->selectionRange();
    m_document->removeText(rangeText);
}

void WorksheetTextEditorItem::copy()
{
    KTextEditor::Range range;
    if (m_view->selectionRange().isValid())
    {
        range = m_view->selectionRange();
    }
    else
    {
        int lastLine = m_document->lines() - 1;
        int lastColumn = m_document->lineLength(lastLine);
        range = KTextEditor::Range(0, 0, lastLine, lastColumn);
    }

    QString text = m_document->text(range);
    text.replace(QChar::ParagraphSeparator, QLatin1Char('\n'));
    text.replace(QChar::LineSeparator, QLatin1Char('\n'));

    QApplication::clipboard()->setText(text);
}

void WorksheetTextEditorItem::paste()
{
    KTextEditor::Cursor cursor = m_view->cursorPosition();
    m_document->insertText(cursor, QGuiApplication::clipboard()->text());
}

void WorksheetTextEditorItem::selectionChanged()
{
    bool hasSelection = m_view->selectionRange().isValid();
    Q_EMIT copyAvailable(hasSelection);

    if(isEditable())
        Q_EMIT cutAvailable(hasSelection);
    if (isEditable())
    Q_EMIT pasteAvailable(!QApplication::clipboard()->text().isEmpty());
}

void WorksheetTextEditorItem::testSize()
{
    qreal currentContentWidth = m_view->contentsRect().width();
    if (currentContentWidth <= 0)
    {
        return;
    }

    QSizeF requiredSize = estimateContentSize(currentContentWidth);

    if (requiredSize.height() != m_size.height() || requiredSize.width() != m_size.width())
    {
        m_size = requiredSize;
        Q_EMIT sizeChanged();
    }

    resetScrollPosition();

    qreal newWidth = scenePos().x() + m_size.width() - 10;
    worksheet()->setRequestedWidth(this, newWidth);
}

void WorksheetTextEditorItem::clipboardChanged()
{
    if(isEditable())
        Q_EMIT pasteAvailable(!QApplication::clipboard()->text().isEmpty());
}

bool WorksheetTextEditorItem::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        if (object == m_customCompleter)
        {
            if (m_customCompleter->isVisible())
            {
                QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
                switch (keyEvent->key())
                {
                    case Qt::Key_Up:
                    case Qt::Key_Down:
                    {
                        int count = m_customCompleter->count();
                        if (count > 0) {
                            int currentRow = m_customCompleter->currentRow();
                            currentRow += (keyEvent->key() == Qt::Key_Up) ? -1 : 1;
                            m_customCompleter->setCurrentRow((currentRow + count) % count);
                        }
                        return true;
                    }
                    case Qt::Key_Enter:
                    case Qt::Key_Return:
                    case Qt::Key_Tab:
                        onCompleterItemSelected();
                        return true;

                    case Qt::Key_Space:
                    case Qt::Key_Escape:
                    case Qt::Key_Backspace:
                        if (m_customCompleter && m_customCompleter->isVisible())
                        {
                            m_customCompleter->hide();
                        }
                        return true;
                    default:
                    {
                        const QString& textToInsert = keyEvent->text();
                        if (!textToInsert.isEmpty()) {
                            m_document->insertText(m_view->cursorPosition(), textToInsert);

                            KTextEditor::Range range = m_view->document()->wordRangeAt(m_view->cursorPosition());
                            m_completionModel->completionInvoked(m_view, range, KTextEditor::CodeCompletionModel::AutomaticInvocation);
                        }
                        return true;
                    }
                }
            }
        }

        else if (object == m_view)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

            bool completionActive = m_view->isCompletionActive();
            bool customCompleterVisible = m_customCompleter && m_customCompleter->isVisible();

            if (completionActive || customCompleterVisible)
            {

                if (completionActive)
                {
                    m_view->abortCompletion();
                }

                if (customCompleterVisible) {
                    if (m_customCompleter && m_customCompleter->isVisible())
                    {
                        m_customCompleter->hide();
                    }
                }
            }
            return false;
        }
    }
    else if (event->type() == QEvent::MouseButtonPress)
    {
        if (m_customCompleter && m_customCompleter->isVisible())
        {
            m_customCompleter->hide();
        }
    }

    return QGraphicsProxyWidget::eventFilter(object, event);
}

void WorksheetTextEditorItem::keyPressEvent(QKeyEvent *event)
{
    worksheet()->resetEntryCursor();

    if (!m_view)
    {
        QGraphicsProxyWidget::keyPressEvent(event);
        return;
    }

    if (!m_view->hasFocus())
    {
        m_view->setFocus();
    }

    if (event->modifiers() == Qt::NoModifier ||((event->modifiers() & Qt::ShiftModifier) && !(event->modifiers() & ~Qt::ShiftModifier)))
    {
        const KTextEditor::Cursor cursor = m_view->cursorPosition();
        switch (event->key())
        {
            case Qt::Key_Up:
                if (cursor.line() == 0)
                {
                    Q_EMIT moveToPrevious(BottomRight, 0);
                    event->accept();
                    return;
                }
                break;
            case Qt::Key_Left:
                if (cursor.line() == 0 && cursor.column() == 0)
                {
                    Q_EMIT moveToPrevious(BottomRight, 0);
                    event->accept();
                    return;
                }
                break;
            case Qt::Key_Down:
                if (cursor.line() == m_document->lines() - 1)
                {
                    Q_EMIT moveToNext(TopLeft, 0);
                    event->accept();
                    return;
                }
                break;
            case Qt::Key_Right:
                if (cursor == m_document->documentEnd())
                {
                    Q_EMIT moveToNext(TopLeft, 0);
                    event->accept();
                    return;
                }
                break;
        }
    }

    switch (event->key())
    {
        case Qt::Key_Home:
            {
                KTextEditor::Range selection = m_view->selectionRange();
                if(selection.isValid())
                    m_view->removeSelection();

                KTextEditor::Cursor cursor = m_view->cursorPosition();
                if (event->modifiers() & Qt::ControlModifier) {
                    m_view->setCursorPosition(KTextEditor::Cursor(0, 0));
                }
                else if (event->modifiers() & Qt::ShiftModifier)
                {
                    KTextEditor::Range selection = m_view->selectionRange();
                    if (!selection.isValid())
                    {
                        m_view->setSelection(KTextEditor::Range(cursor.line(), 0, cursor.line(), cursor.column()));
                    }
                    else
                    {
                        KTextEditor::Cursor start = selection.start();
                        KTextEditor::Cursor end = selection.end();
                        if (cursor == end)
                        {
                            m_view->setSelection(KTextEditor::Range(start, KTextEditor::Cursor(cursor.line(), 0)));
                        }
                        else {

                            m_view->setSelection(KTextEditor::Range(KTextEditor::Cursor(cursor.line(), 0), end));
                        }
                    }
                }
                else
                {
                    m_view->setCursorPosition(KTextEditor::Cursor(cursor.line(), 0));
                }
                event->accept();
                return;
            }
        case Qt::Key_End:
            {
                KTextEditor::Range selection = m_view->selectionRange();
                if(selection.isValid())
                    m_view->removeSelection();

                KTextEditor::Cursor cursor = m_view->cursorPosition();
                int lineLength = m_document->lineLength(cursor.line());

                if (event->modifiers() & Qt::ControlModifier)
                {
                    int lastLine = m_document->lines() - 1;
                    int lastLineLength = m_document->lineLength(lastLine);
                    m_view->setCursorPosition(KTextEditor::Cursor(lastLine, lastLineLength));
                }
                else if (event->modifiers() & Qt::ShiftModifier)
                {
                    KTextEditor::Range selection = m_view->selectionRange();
                    if (!selection.isValid())
                    {
                        m_view->setSelection(KTextEditor::Range(cursor.line(), cursor.column(), cursor.line(), lineLength));
                    }
                    else
                    {
                        KTextEditor::Cursor start = selection.start();
                        KTextEditor::Cursor end = selection.end();
                        if (cursor == end)
                        {
                            m_view->setSelection(KTextEditor::Range(start, KTextEditor::Cursor(cursor.line(), lineLength)));
                        }
                        else
                        {
                            m_view->setSelection(KTextEditor::Range(KTextEditor::Cursor(cursor.line(), lineLength), end));
                        }
                    }
                }
                else
                {
                    m_view->setCursorPosition(KTextEditor::Cursor(cursor.line(), lineLength));
                }
                event->accept();
                return;
            }
        case Qt::Key_Left:
        {
            KTextEditor::Range selection = m_view->selectionRange();
            if(selection.isValid())
                m_view->removeSelection();

            if (event->modifiers() == Qt::NoModifier || event->modifiers() == Qt::ShiftModifier)
            {
                KTextEditor::Cursor cursor = m_view->cursorPosition();
                KTextEditor::Cursor newCursor = cursor;

                if (cursor.column() > 0)
                {
                    newCursor = KTextEditor::Cursor(cursor.line(), cursor.column() - 1);
                }
                else if (cursor.line() > 0)
                {
                    int prevLineLength = m_document->lineLength(cursor.line() - 1);
                    newCursor = KTextEditor::Cursor(cursor.line() - 1, prevLineLength);
                }

                if (event->modifiers() & Qt::ShiftModifier)
                {
                    KTextEditor::Range selection = m_view->selectionRange();
                    if (!selection.isValid())
                    {
                        m_view->setSelection(KTextEditor::Range(newCursor, cursor));
                        m_view->setCursorPosition(newCursor);
                    }
                    else
                    {
                        KTextEditor::Cursor start = selection.start();
                        KTextEditor::Cursor end = selection.end();
                        if (cursor == end) {
                            m_view->setSelection(KTextEditor::Range(start, newCursor));
                            m_view->setCursorPosition(newCursor);
                        }
                        else
                        {
                            m_view->setSelection(KTextEditor::Range(newCursor, end));
                            m_view->setCursorPosition(newCursor);
                        }
                    }
                }
                else
                {
                    m_view->setCursorPosition(newCursor);
                }
                event->accept();
                return;
            }
            break;
        }
        case Qt::Key_Right:
        {
            KTextEditor::Range selection = m_view->selectionRange();
            if(selection.isValid())
                m_view->removeSelection();

            if (event->modifiers() == Qt::NoModifier || event->modifiers() == Qt::ShiftModifier)
            {
                KTextEditor::Cursor cursor = m_view->cursorPosition();
                KTextEditor::Cursor newCursor = cursor;

                if (cursor.column() < m_document->lineLength(cursor.line()))
                {
                    newCursor = KTextEditor::Cursor(cursor.line(), cursor.column() + 1);
                }
                else if (cursor.line() < m_document->lines() - 1)
                {
                    newCursor = KTextEditor::Cursor(cursor.line() + 1, 0);
                }

                if (event->modifiers() & Qt::ShiftModifier) {
                    KTextEditor::Range selection = m_view->selectionRange();
                    if (!selection.isValid())
                    {
                        m_view->setSelection(KTextEditor::Range(cursor, newCursor));
                        m_view->setCursorPosition(newCursor);
                    }
                    else
                    {
                        KTextEditor::Cursor start = selection.start();
                        KTextEditor::Cursor end = selection.end();
                        if (cursor == end)
                        {
                            m_view->setSelection(KTextEditor::Range(start, newCursor));
                            m_view->setCursorPosition(newCursor);
                        }
                        else
                        {
                            m_view->setSelection(KTextEditor::Range(newCursor, end));
                            m_view->setCursorPosition(newCursor);
                        }
                    }
                }
                else
                {
                    m_view->setCursorPosition(newCursor);
                }
                event->accept();
                return;
            }
            break;
        }
        case Qt::Key_Up:
        {
            KTextEditor::Range selection = m_view->selectionRange();
            if(selection.isValid())
                m_view->removeSelection();

            if (event->modifiers() == Qt::NoModifier || event->modifiers() == Qt::ShiftModifier)
            {
                KTextEditor::Cursor cursor = m_view->cursorPosition();
                KTextEditor::Cursor newCursor = cursor;

                if (cursor.line() > 0)
                {
                    int targetColumn = qMin(cursor.column(), m_document->lineLength(cursor.line() - 1));
                    newCursor = KTextEditor::Cursor(cursor.line() - 1, targetColumn);
                }

                if (event->modifiers() & Qt::ShiftModifier)
                {
                    KTextEditor::Range selection = m_view->selectionRange();
                    if (!selection.isValid()) {
                        m_view->setSelection(KTextEditor::Range(newCursor, cursor));
                        m_view->setCursorPosition(newCursor);
                    }
                    else
                    {
                        KTextEditor::Cursor start = selection.start();
                        KTextEditor::Cursor end = selection.end();
                        if (cursor == end)
                        {
                            m_view->setSelection(KTextEditor::Range(start, newCursor));
                            m_view->setCursorPosition(newCursor);
                        }
                        else
                        {
                            m_view->setSelection(KTextEditor::Range(newCursor, end));
                            m_view->setCursorPosition(newCursor);
                        }
                    }
                }
                else
                {
                    m_view->setCursorPosition(newCursor);
                }
                event->accept();
                return;
            }
            break;
        }
        case Qt::Key_Down:
        {
            KTextEditor::Range selection = m_view->selectionRange();
            if(selection.isValid())
                m_view->removeSelection();

            if (event->modifiers() == Qt::NoModifier || event->modifiers() == Qt::ShiftModifier)
            {
                KTextEditor::Cursor cursor = m_view->cursorPosition();
                KTextEditor::Cursor newCursor = cursor;

                if (cursor.line() < m_document->lines() - 1)
                {
                    int targetColumn = qMin(cursor.column(), m_document->lineLength(cursor.line() + 1));
                    newCursor = KTextEditor::Cursor(cursor.line() + 1, targetColumn);
                }

                if (event->modifiers() & Qt::ShiftModifier)
                {
                    KTextEditor::Range selection = m_view->selectionRange();
                    if (!selection.isValid())
                    {
                        m_view->setSelection(KTextEditor::Range(cursor, newCursor));
                        m_view->setCursorPosition(newCursor);
                    }
                    else
                    {
                        KTextEditor::Cursor start = selection.start();
                        KTextEditor::Cursor end = selection.end();
                        if (cursor == end)
                        {
                            m_view->setSelection(KTextEditor::Range(start, newCursor));
                            m_view->setCursorPosition(newCursor);
                        }
                        else
                        {
                            m_view->setSelection(KTextEditor::Range(newCursor, end));
                            m_view->setCursorPosition(newCursor);
                        }
                    }
                }
                else
                {
                    m_view->setCursorPosition(newCursor);
                }
                event->accept();
                return;
            }
            break;
        }
        case Qt::Key_Backspace:

            {
                bool textDeleted = false;

                KTextEditor::Range selection = m_view->selectionRange();
                if (selection.isValid())
                {
                    textDeleted = m_document->removeText(selection);
                    m_view->removeSelection();
                }
                else
                {
                    KTextEditor::Cursor cursor = m_view->cursorPosition();
                    if (cursor.column() > 0)
                    {
                        KTextEditor::Range range(cursor.line(), cursor.column() - 1, cursor.line(), cursor.column());
                        textDeleted = m_document->removeText(range);
                    }
                    else if (cursor.line() > 0)
                    {
                        int prevLineLength = m_document->lineLength(cursor.line() - 1);
                        KTextEditor::Range range(cursor.line() - 1, prevLineLength, cursor.line(), 0);
                        textDeleted = m_document->removeText(range);
                    }
                }

                if (textDeleted) {
                    QTimer::singleShot(0, this, [this]()
                    {
                        if (m_view && m_view->hasFocus() && m_completionModel)
                        {
                            KTextEditor::Cursor cursor = m_view->cursorPosition();
                            QString line = m_document->line(cursor.line());
                            QString textBeforeCursor = line.left(cursor.column());

                            if (!textBeforeCursor.trimmed().isEmpty()) {
                                KTextEditor::Range range = m_view->document()->wordRangeAt(cursor);
                                m_completionModel->completionInvoked(m_view, range, KTextEditor::CodeCompletionModel::AutomaticInvocation);
                            }
                        }
                    });
                }

                event->accept();
                return;
            }
        case Qt::Key_Delete:
            {
                bool textDeleted = false;

                KTextEditor::Range selection = m_view->selectionRange();
                if (selection.isValid())
                {
                    textDeleted = m_document->removeText(selection);
                    m_view->removeSelection();
                }
                else
                {
                    KTextEditor::Cursor cursor = m_view->cursorPosition();
                    if (cursor.column() < m_document->lineLength(cursor.line()))
                    {
                        KTextEditor::Range range(cursor.line(), cursor.column(), cursor.line(), cursor.column() + 1);
                        textDeleted = m_document->removeText(range);
                    }
                    else if (cursor.line() < m_document->lines() - 1)
                    {
                        KTextEditor::Range range(cursor.line(), cursor.column(), cursor.line() + 1, 0);
                        textDeleted = m_document->removeText(range);
                    }
                }
                if (textDeleted)
                {
                    QTimer::singleShot(0, this, [this]()
                    {
                        if (m_view && m_view->hasFocus() && m_completionModel)
                        {
                            KTextEditor::Cursor cursor = m_view->cursorPosition();
                            QString line = m_document->line(cursor.line());
                            QString textBeforeCursor = line.left(cursor.column());

                            if (!textBeforeCursor.trimmed().isEmpty())
                            {
                                KTextEditor::Range range = m_view->document()->wordRangeAt(cursor);
                                m_completionModel->completionInvoked(m_view, range, KTextEditor::CodeCompletionModel::AutomaticInvocation);
                            }
                        }
                    });
                }

                event->accept();
                return;
            }
    }

    QGraphicsProxyWidget::keyPressEvent(event);
}

void WorksheetTextEditorItem::focusInEvent(QFocusEvent *event)
{
    worksheet()->resetEntryCursor();
    if (m_view) {
        m_view->setFocus(event->reason());
    }
    QGraphicsProxyWidget::focusInEvent(event);
    worksheet()->updateFocusedTextItem(this);
    Q_EMIT receivedFocus(this);
}

void WorksheetTextEditorItem::focusOutEvent(QFocusEvent *event)
{
    QApplication::activePopupWidget() ;
    QGraphicsProxyWidget::focusOutEvent(event);
    Q_EMIT cursorPositionChanged(KTextEditor::Cursor::invalid());
}

void WorksheetTextEditorItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsProxyWidget::mousePressEvent(event);
    KTextEditor::View* view = m_view;

    KTextEditor::Cursor cursor = view->cursorPosition();
    bool hadSelectionBefore = view->selectionRange().isValid();

    if(event->button() == Qt::MiddleButton && QApplication::clipboard()->supportsSelection() && !event->isAccepted())
    {
        event->accept();
    }
    if(m_dragEnabled && event->button() == Qt::LeftButton)
    {
        event->accept();
    }

    KTextEditor::Cursor cursorAfter = view->cursorPosition();
    bool hasSelectionAfter = view->selectionRange().isValid();

    if(cursor != cursorAfter)
        Q_EMIT cursorPositionChanged(cursorAfter);
    if(hadSelectionBefore != hasSelectionAfter)
        selectionChanged();
}

void WorksheetTextEditorItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    KTextEditor::View* view = m_view;
    KTextEditor::Cursor oldCursor;
    if (view)
        oldCursor = view->cursorPosition();

    if (isEditable() && event->button() == Qt::MiddleButton &&
        QApplication::clipboard()->supportsSelection())
    {

        QWidget* widget = this->widget();
        if (widget)
        {
            QPointF proxyLocalPos = this->mapFromScene(event->scenePos());
            QPoint widgetPoint = widget->mapToGlobal(proxyLocalPos.toPoint());

            QMouseEvent mouseEvent(QEvent::MouseButtonRelease, widgetPoint, Qt::MiddleButton,
                                   Qt::MiddleButton, Qt::NoModifier);
            QCoreApplication::sendEvent(widget, &mouseEvent);
        }

        const QString& text = QApplication::clipboard()->text(QClipboard::Selection);
        if (view)
        {
            KTextEditor::Cursor pos = view->cursorPosition();
            view->document()->replaceText(KTextEditor::Range(pos, pos), text);
        }

        } else
        {
            QGraphicsProxyWidget::mouseReleaseEvent(event);
        }

        if (view && oldCursor != view->cursorPosition())
            Q_EMIT cursorPositionChanged(view->cursorPosition());
}

void WorksheetTextEditorItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu *menu = worksheet()->createContextMenu();
    populateMenu(menu, event->pos());

    menu->popup(event->screenPos());
}

void WorksheetTextEditorItem::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    if (isEditable() && event->mimeData()->hasFormat(QLatin1String("text/plain")))
    {
        QPoint localPos = mapFromScene(event->scenePos()).toPoint();

        KTextEditor::Cursor cursor = m_view->coordinatesToCursor(localPos);
        if(cursor.isValid())
        {
            m_view->setCursorPosition(cursor);
            Q_EMIT cursorPositionChanged(cursor);
        }
    }
}

void WorksheetTextEditorItem::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    if (isEditable())
    {
        KTextEditor::Cursor cursor = m_view->cursorPosition();
        if (event->mimeData()->hasText())
        {
            QString text = event->mimeData()->text();
            m_document->insertText(cursor, text);
        }
        event->accept();
    }
}


void WorksheetTextEditorItem::showCustomCompleter(const QList<CantorCompletionModel::CompletionItem>& matches)
{
    if (matches.isEmpty())
    {
        if (m_customCompleter && m_customCompleter->isVisible())
        {
            m_customCompleter->hide();
        }
        return;
    }

    if (!m_customCompleter)
    {
        m_customCompleter = new QListWidget();
        m_customCompleter->setWindowFlags(Qt::Popup);
        m_customCompleter->setFocusPolicy(Qt::NoFocus);
        m_customCompleter->installEventFilter(this);

        connect(m_customCompleter, &QListWidget::itemActivated, this, &WorksheetTextEditorItem::onCompleterItemSelected);
    }

    m_customCompleter->clear();
    for (const auto& item : matches)
    {
        m_customCompleter->addItem(item.name);
    }
    m_customCompleter->setCurrentRow(0);

    QRectF localCursorRect = cursorRect();
    QPointF scenePos = mapToScene(localCursorRect.bottomLeft());
    QPoint globalPos = worksheetView()->viewport()->mapToGlobal(worksheetView()->mapFromScene(scenePos));

    m_customCompleter->move(globalPos);
    m_customCompleter->show();
}

void WorksheetTextEditorItem::onCompleterItemSelected()
{
    if (!m_customCompleter || !m_customCompleter->currentItem() || !m_completionModel)
    {
        return;
    }

    int currentIndex = m_customCompleter->currentRow();

    QModelIndex modelIndex = m_completionModel->index(currentIndex, 0);

    if (modelIndex.isValid())
    {
        m_completionModel->executeCompletionItem(m_view, KTextEditor::Range(), modelIndex);
    }

    if (m_customCompleter && m_customCompleter->isVisible())
    {
        m_customCompleter->hide();
    }
    m_view->setFocus();
}

void WorksheetTextEditorItem::hideCompleterAndResetFocus()
{
    if (m_customCompleter && m_customCompleter->isVisible())
    {
        m_customCompleter->hide();
    }
    if (m_view)
        m_view->setFocus();
}

bool WorksheetTextEditorItem::sceneEvent(QEvent *event)
{

    if(event->type() == QEvent::ShortcutOverride)
    {
        QKeyEvent* kev = static_cast<QKeyEvent*>(event);
        if (kev->matches(QKeySequence::Find) || kev->matches(QKeySequence::Replace))
        {
            kev->accept();
            return true;
        }
        if (kev->matches(QKeySequence::SelectAll))
        {
             kev->accept();
             KTextEditor::Range fullRange(KTextEditor::Cursor(0, 0), m_view->document()->documentEnd());
             m_view->setSelection(fullRange);
             return true;
        }
        QKeySequence sqe(kev->key() | kev->modifiers());
        if(worksheet()->isShortcut(sqe))
        {
            kev->ignore();
            return false;
        }
    }
    return QGraphicsProxyWidget::sceneEvent(event);
}

void WorksheetTextEditorItem::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    QGraphicsProxyWidget::wheelEvent(event);
    event->ignore();
}

QPointF WorksheetTextEditorItem::localCursorPosition() const
{
    KTextEditor::Cursor Cursor = m_view->cursorPosition();
    QPoint viewLocalPos = m_view->cursorToCoordinate(Cursor);

    QPointF proxyPos = this->subWidgetRect(m_view).topLeft() + viewLocalPos;

    return proxyPos;
}

Cantor::Session* WorksheetTextEditorItem::session()
{
    return worksheet()->session();
}

QKeyEvent* WorksheetTextEditorItem::eventForStandardAction(KStandardAction::StandardAction actionID)
{
    auto* action = KStandardAction::create(actionID, this, &WorksheetTextEditorItem::copy, this);
    QKeySequence keySeq = action->shortcut();

    int code = keySeq[0];
    const int ModMask = Qt::ShiftModifier | Qt::ControlModifier |
    Qt::AltModifier | Qt::MetaModifier;
    const int KeyMask = ~ModMask;
    QKeyEvent* event = new QKeyEvent(QEvent::KeyPress, code & KeyMask,QFlags<Qt::KeyboardModifier>(code & ModMask));
    delete action;
    return event;
}

void WorksheetTextEditorItem::resetScrollPosition()
{
    if (m_view && m_view->verticalScrollBar())
    {
        m_view->verticalScrollBar()->setValue(0);
    }
}
