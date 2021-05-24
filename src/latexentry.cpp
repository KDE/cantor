/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#include "latexentry.h"

#include "worksheetentry.h"
#include "worksheet.h"
#include "lib/renderer.h"
#include "lib/jupyterutils.h"
#include "lib/defaulthighlighter.h"
#include "lib/latexrenderer.h"
#include "config-cantor.h"

#include <QTextCursor>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QBuffer>
#include <QUuid>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>

#include <KZip>
#include <KLocalizedString>

LatexEntry::LatexEntry(Worksheet* worksheet) : WorksheetEntry(worksheet), m_textItem(new WorksheetTextItem(this, Qt::TextEditorInteraction))
{
    m_textItem->installEventFilter(this);

    connect(m_textItem, &WorksheetTextItem::moveToPrevious, this, &LatexEntry::moveToPreviousEntry);
    connect(m_textItem, &WorksheetTextItem::moveToNext, this, &LatexEntry::moveToNextEntry);
    connect(m_textItem, SIGNAL(execute()), this, SLOT(evaluate()));
}

void LatexEntry::populateMenu(QMenu* menu, QPointF pos)
{
    bool imageSelected = false;
    QTextCursor cursor = m_textItem->textCursor();
    const QChar repl = QChar::ObjectReplacementCharacter;
    if (cursor.hasSelection()) {
        QString selection = m_textItem->textCursor().selectedText();
        imageSelected = selection.contains(repl);
    } else {
        // we need to try both the current cursor and the one after the that
        cursor = m_textItem->cursorForPosition(pos);
        for (int i = 2; i; --i) {
            int p = cursor.position();
            if (m_textItem->document()->characterAt(p-1) == repl &&
                cursor.charFormat().hasProperty(Cantor::Renderer::CantorFormula)) {
                m_textItem->setTextCursor(cursor);
                imageSelected = true;
                break;
            }
            cursor.movePosition(QTextCursor::NextCharacter);
        }
    }
    if (imageSelected) {
        menu->addAction(i18n("Show LaTeX code"), this, SLOT(resolveImagesAtCursor()));
        menu->addSeparator();
    }
    WorksheetEntry::populateMenu(menu, pos);
}

int LatexEntry::type() const
{
    return Type;
}

bool LatexEntry::isEmpty()
{
    return m_textItem->document()->isEmpty();
}

bool LatexEntry::acceptRichText()
{
    return false;
}

bool LatexEntry::focusEntry(int pos, qreal xCoord)
{
    if (aboutToBeRemoved())
        return false;
    m_textItem->setFocusAt(pos, xCoord);
    return true;
}

void LatexEntry::setContent(const QString& content)
{
    m_latex = content;
    m_textItem->setPlainText(m_latex);
}

void LatexEntry::setContent(const QDomElement& content, const KZip& file)
{
    m_latex = content.text();
    qDebug() << m_latex;

    m_textItem->document()->clear();
    QTextCursor cursor = m_textItem->textCursor();
    cursor.movePosition(QTextCursor::Start);

    QString imagePath;
    bool useLatexCode = true;

    if(content.hasAttribute(QLatin1String("filename")))
    {
        const KArchiveEntry* imageEntry=file.directory()->entry(content.attribute(QLatin1String("filename")));
        if (imageEntry&&imageEntry->isFile())
        {
            const KArchiveFile* imageFile=static_cast<const KArchiveFile*>(imageEntry);
            const QString& dir=QStandardPaths::writableLocation(QStandardPaths::TempLocation);
            imageFile->copyTo(dir);
            imagePath = dir + QDir::separator() + imageFile->name();

#ifdef LIBSPECTRE_FOUND
            QString uuid = Cantor::LatexRenderer::genUuid();
            m_renderedFormat = worksheet()->renderer()->render(m_textItem->document(), Cantor::Renderer::EPS, QUrl::fromLocalFile(imagePath), uuid);
            qDebug()<<"rendering successful? " << !m_renderedFormat.name().isEmpty();

            m_renderedFormat.setProperty(Cantor::Renderer::CantorFormula, Cantor::Renderer::LatexFormula);
            m_renderedFormat.setProperty(Cantor::Renderer::ImagePath, imagePath);
            m_renderedFormat.setProperty(Cantor::Renderer::Code, m_latex);

            cursor.insertText(QString(QChar::ObjectReplacementCharacter), m_renderedFormat);
            useLatexCode = false;
            m_textItem->denyEditing();
#endif
        }
    }

    if (useLatexCode && content.hasAttribute(QLatin1String("image")))
    {
        const QByteArray& ba = QByteArray::fromBase64(content.attribute(QLatin1String("image")).toLatin1());
        QImage image;
        if (image.loadFromData(ba))
        {
            // Create unique internal url for this loaded image
            QUrl internal;
            internal.setScheme(QLatin1String("internal"));
            internal.setPath(QUuid::createUuid().toString());

            m_textItem->document()->addResource(QTextDocument::ImageResource, internal, QVariant(image));

            m_renderedFormat.setName(internal.url());
            m_renderedFormat.setWidth(image.width());
            m_renderedFormat.setHeight(image.height());

            m_renderedFormat.setProperty(Cantor::Renderer::CantorFormula, Cantor::Renderer::LatexFormula);
            if (!imagePath.isEmpty())
                m_renderedFormat.setProperty(Cantor::Renderer::ImagePath, imagePath);
            m_renderedFormat.setProperty(Cantor::Renderer::Code, m_latex);

            cursor.insertText(QString(QChar::ObjectReplacementCharacter), m_renderedFormat);
            useLatexCode = false;
            m_textItem->denyEditing();
        }
    }

    if (useLatexCode)
        cursor.insertText(m_latex);
}

void LatexEntry::setContentFromJupyter(const QJsonObject& cell)
{
    if (!Cantor::JupyterUtils::isCodeCell(cell))
        return;

    m_textItem->document()->clear();
    QTextCursor cursor = m_textItem->textCursor();
    cursor.movePosition(QTextCursor::Start);

    bool useLatexCode = true;

    QString source = Cantor::JupyterUtils::getSource(cell);
    m_latex = source.remove(QLatin1String("%%latex\n"));

    QJsonArray outputs = cell.value(Cantor::JupyterUtils::outputsKey).toArray();
    if (outputs.size() == 1 && Cantor::JupyterUtils::isJupyterDisplayOutput(outputs[0]))
    {
        const QJsonObject data = outputs[0].toObject().value(Cantor::JupyterUtils::dataKey).toObject();
        const QImage& image = Cantor::JupyterUtils::loadImage(data, Cantor::JupyterUtils::pngMime);
        if (!image.isNull())
        {
            QUrl internal;
            internal.setScheme(QLatin1String("internal"));
            internal.setPath(QUuid::createUuid().toString());

            m_textItem->document()->addResource(QTextDocument::ImageResource, internal, QVariant(image));

            m_renderedFormat.setName(internal.url());
            m_renderedFormat.setWidth(image.width());
            m_renderedFormat.setHeight(image.height());

            m_renderedFormat.setProperty(Cantor::Renderer::CantorFormula, Cantor::Renderer::LatexFormula);
            m_renderedFormat.setProperty(Cantor::Renderer::Code, m_latex);

            cursor.insertText(QString(QChar::ObjectReplacementCharacter), m_renderedFormat);
            useLatexCode = false;
            m_textItem->denyEditing();
        }
    }

    if (useLatexCode)
    {
        cursor.insertText(m_latex);
        m_latex.clear(); // We don't render image, so clear latex code cache
    }
}

QJsonValue LatexEntry::toJupyterJson()
{
    QJsonObject entry;
    entry.insert(Cantor::JupyterUtils::cellTypeKey, QLatin1String("code"));
    entry.insert(Cantor::JupyterUtils::executionCountKey, QJsonValue());

    QJsonObject metadata, cantorMetadata;
    cantorMetadata.insert(QLatin1String("latex_entry"), true);
    metadata.insert(Cantor::JupyterUtils::cantorMetadataKey, cantorMetadata);
    entry.insert(Cantor::JupyterUtils::metadataKey, metadata);

    QJsonArray outputs;

    QTextCursor cursor = m_textItem->document()->find(QString(QChar::ObjectReplacementCharacter));
    if (!cursor.isNull())
    {
        QTextImageFormat format=cursor.charFormat().toImageFormat();

        QUrl internal;
        internal.setUrl(format.name());
        const QImage& image = m_textItem->document()->resource(QTextDocument::ImageResource, internal).value<QImage>();
        if (!image.isNull())
        {
            QByteArray ba;
            QBuffer buffer(&ba);
            buffer.open(QIODevice::WriteOnly);
            image.save(&buffer, "PNG");

            // Add image result with latex rendered image to this Jupyter code cell
            QJsonObject imageResult;
            imageResult.insert(Cantor::JupyterUtils::outputTypeKey, QLatin1String("display_data"));

            QJsonObject data;
            data.insert(Cantor::JupyterUtils::pngMime, Cantor::JupyterUtils::toJupyterMultiline(QString::fromLatin1(ba.toBase64())));
            imageResult.insert(QLatin1String("data"), data);

            imageResult.insert(Cantor::JupyterUtils::metadataKey, QJsonObject());

            outputs.append(imageResult);
        }
    }
    entry.insert(Cantor::JupyterUtils::outputsKey, outputs);

    const QString& latex = latexCode();
    Cantor::JupyterUtils::setSource(entry, QLatin1String("%%latex\n") + latex);

    return entry;
}

QDomElement LatexEntry::toXml(QDomDocument& doc, KZip* archive)
{
    QDomElement el = doc.createElement(QLatin1String("Latex"));
    el.appendChild( doc.createTextNode( latexCode() ));

    QTextCursor cursor = m_textItem->document()->find(QString(QChar::ObjectReplacementCharacter));
    if (!cursor.isNull())
    {
        QTextImageFormat format=cursor.charFormat().toImageFormat();
        QString fileName = format.property(Cantor::Renderer::ImagePath).toString();
        // Check, if eps file exists, and if not true, rerender latex code
        bool isEpsFileExists = QFile::exists(fileName);

#ifdef LIBSPECTRE_FOUND
        if (!isEpsFileExists && renderLatexCode())
        {
            cursor = m_textItem->document()->find(QString(QChar::ObjectReplacementCharacter));
            format=cursor.charFormat().toImageFormat();
            fileName = format.property(Cantor::Renderer::ImagePath).toString();
            isEpsFileExists = QFile::exists(fileName);
        }
#endif

        if (isEpsFileExists && archive)
        {
            const QUrl& url=QUrl::fromLocalFile(fileName);
            archive->addLocalFile(url.toLocalFile(), url.fileName());
            el.setAttribute(QLatin1String("filename"), url.fileName());
        }

        // Save also rendered QImage, if exist.
        QUrl internal;
        internal.setUrl(format.name());

        const QImage& image = m_textItem->document()->resource(QTextDocument::ImageResource, internal).value<QImage>();
        if (!image.isNull())
        {
            QByteArray ba;
            QBuffer buffer(&ba);
            buffer.open(QIODevice::WriteOnly);
            image.save(&buffer, "PNG");
            el.setAttribute(QLatin1String("image"), QString::fromLatin1(ba.toBase64()));
        }
    }

    return el;
}

QString LatexEntry::toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq)
{
    Q_UNUSED(commandSep);

    if (commentStartingSeq.isEmpty())
        return QString();

    QString text = latexCode();
    if (!commentEndingSeq.isEmpty())
        return commentStartingSeq + text + commentEndingSeq + QLatin1String("\n");
    return commentStartingSeq + text.replace(QLatin1String("\n"), QLatin1String("\n") + commentStartingSeq) + QLatin1String("\n");
}

bool LatexEntry::evaluate(EvaluationOption evalOp)
{
    bool success = false;

    if (isOneImageOnly())
    {
        success = true;
    }
    else
    {
        if (m_latex == latexCode())
        {
            bool renderWasSuccessful = !m_renderedFormat.name().isEmpty();
            if (renderWasSuccessful)
            {
                QTextCursor cursor = m_textItem->textCursor();
                cursor.movePosition(QTextCursor::Start);
                cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
                cursor.insertText(QString(QChar::ObjectReplacementCharacter), m_renderedFormat);
                m_textItem->denyEditing();
            }
            else
            {
                success = renderLatexCode();
            }
        }
        else
        {
            m_latex = latexCode();
            success = renderLatexCode();
        }
    }

    qDebug()<<"rendering successful? "<<success;

    evaluateNext(evalOp);
    return success;
}

void LatexEntry::updateEntry()
{
    QTextCursor cursor = m_textItem->document()->find(QString(QChar::ObjectReplacementCharacter));
    while (!cursor.isNull())
    {
        qDebug()<<"found a formula... rendering the eps...";
        QTextImageFormat format=cursor.charFormat().toImageFormat();
        const QUrl& url=QUrl::fromLocalFile(format.property(Cantor::Renderer::ImagePath).toString());
        QSizeF s = worksheet()->renderer()->renderToResource(m_textItem->document(), Cantor::Renderer::EPS, url, QUrl(format.name()));
        qDebug()<<"rendering successful? "<< s.isValid();

        cursor.movePosition(QTextCursor::NextCharacter);

        cursor = m_textItem->document()->find(QString(QChar::ObjectReplacementCharacter), cursor);
    }
}

bool LatexEntry::eventFilter(QObject* object, QEvent* event)
{
    if(object == m_textItem)
    {
        if (event->type() == QEvent::GraphicsSceneMouseDoubleClick)
        {
            // One image if we have rendered entry
            if (isOneImageOnly())
            {
                QTextCursor cursor = m_textItem->textCursor();
                if (!cursor.hasSelection())
                    cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);

                cursor.insertText(m_textItem->resolveImages(cursor));
                m_textItem->allowEditing();
                return true;
            }
        }
        else if (event->type() == QEvent::KeyPress)
        {
            auto* key_event = static_cast<QKeyEvent*>(event);
            if (key_event->matches(QKeySequence::Cancel))
            {
                QTextCursor cursor = m_textItem->textCursor();
                cursor.movePosition(QTextCursor::Start);
                cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
                cursor.insertText(QString(QChar::ObjectReplacementCharacter), m_renderedFormat);
                m_textItem->denyEditing();
                return true;
            }
        }
    }
    return false;
}

QString LatexEntry::latexCode()
{
    QTextCursor cursor = m_textItem->textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

    QString code = m_textItem->resolveImages(cursor);
    code.replace(QChar::ParagraphSeparator, QLatin1Char('\n')); //Replace the U+2029 paragraph break by a Normal Newline
    code.replace(QChar::LineSeparator, QLatin1Char('\n')); //Replace the line break by a Normal Newline
    return code;
}

bool LatexEntry::isOneImageOnly()
{
    QTextCursor cursor = m_textItem->textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

    return (cursor.selectionEnd() == 1 && cursor.selectedText() == QString(QChar::ObjectReplacementCharacter));
}

int LatexEntry::searchText(const QString& text, const QString& pattern,
                          QTextDocument::FindFlags qt_flags)
{
    Qt::CaseSensitivity caseSensitivity;
    if (qt_flags & QTextDocument::FindCaseSensitively)
        caseSensitivity = Qt::CaseSensitive;
    else
        caseSensitivity = Qt::CaseInsensitive;

    int position;
    if (qt_flags & QTextDocument::FindBackward)
        position = text.lastIndexOf(pattern, -1, caseSensitivity);
    else
        position = text.indexOf(pattern, 0, caseSensitivity);

    return position;
}

WorksheetCursor LatexEntry::search(const QString& pattern, unsigned flags,
                                   QTextDocument::FindFlags qt_flags,
                                   const WorksheetCursor& pos)
{
    if (!(flags & WorksheetEntry::SearchLaTeX))
        return WorksheetCursor();
    if (pos.isValid() && (pos.entry() != this || pos.textItem() != m_textItem))
        return WorksheetCursor();

    QTextCursor textCursor = m_textItem->search(pattern, qt_flags, pos);
    int position = 0;
    QString latex;
    const QString repl = QString(QChar::ObjectReplacementCharacter);
    QTextCursor latexCursor = m_textItem->search(repl, qt_flags, pos);

    while (!latexCursor.isNull()) {
        latex = m_textItem->resolveImages(latexCursor);
        position = searchText(latex, pattern, qt_flags);
        if (position >= 0) {
            break;
        }
        WorksheetCursor c(this, m_textItem, latexCursor);
        latexCursor = m_textItem->search(repl, qt_flags, c);
    }

    if (latexCursor.isNull()) {
        if (textCursor.isNull())
            return WorksheetCursor();
        else
            return WorksheetCursor(this, m_textItem, textCursor);
    } else {
        if (textCursor.isNull() || latexCursor < textCursor) {
            int start = latexCursor.selectionStart();
            latexCursor.insertText(latex);
            QTextCursor c = m_textItem->textCursor();
            c.setPosition(start + position);
            c.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor,
                           pattern.length());
            return WorksheetCursor(this, m_textItem, c);
        } else {
            return WorksheetCursor(this, m_textItem, textCursor);
        }
    }
}

void LatexEntry::layOutForWidth(qreal entry_zone_x, qreal w, bool force)
{
    if (size().width() == w && m_textItem->pos().x() == entry_zone_x && !force)
        return;

    const qreal margin = worksheet()->isPrinting() ? 0 : RightMargin;

    m_textItem->setGeometry(entry_zone_x, 0, w - margin - entry_zone_x);
    setSize(QSizeF(m_textItem->width() + margin + entry_zone_x, m_textItem->height() + VerticalMargin));
}

bool LatexEntry::wantToEvaluate()
{
    return !isOneImageOnly();
}

bool LatexEntry::renderLatexCode()
{
    bool success = false;
    QString latex = latexCode();
    m_renderedFormat = QTextImageFormat(); // clear rendered image
    Cantor::LatexRenderer* renderer = new Cantor::LatexRenderer(this);
    renderer->setLatexCode(latex);
    renderer->setEquationOnly(false);
    renderer->setMethod(Cantor::LatexRenderer::LatexMethod);
    renderer->renderBlocking();

    if (renderer->renderingSuccessful())
    {
        Cantor::Renderer* epsRend = worksheet()->renderer();
        m_renderedFormat = epsRend->render(m_textItem->document(), renderer);
        success = !m_renderedFormat.name().isEmpty();
    }
    else
        qWarning() << "Fail to render LatexEntry with error " << renderer->errorMessage();

    if(success)
    {
        QTextCursor cursor = m_textItem->textCursor();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        cursor.insertText(QString(QChar::ObjectReplacementCharacter), m_renderedFormat);
        m_textItem->denyEditing();
    }

    delete renderer;
    return success;
}

bool LatexEntry::isConvertableToLatexEntry(const QJsonObject& cell)
{
    if (!Cantor::JupyterUtils::isCodeCell(cell))
        return false;

    const QString& source = Cantor::JupyterUtils::getSource(cell);

    return source.startsWith(QLatin1String("%%latex\n"));
}

void LatexEntry::resolveImagesAtCursor()
{
    QTextCursor cursor = m_textItem->textCursor();
    if (!cursor.hasSelection())
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
    cursor.insertText(m_textItem->resolveImages(cursor));
}

QString LatexEntry::plain() const
{
    return m_textItem->toPlainText();
}
