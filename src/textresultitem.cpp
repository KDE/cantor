/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#include "textresultitem.h"
#include "commandentry.h"
#include "lib/result.h"
#include "lib/textresult.h"
#include "lib/latexresult.h"
#include "lib/renderer.h"
#include "lib/mimeresult.h"
#include "lib/htmlresult.h"
#include "mathrendertask.h"
#include "config-cantor.h"
#include "settings.h"

#include <QDebug>
#include <QFileDialog>
#include <QTextCursor>

#include <KStandardAction>
#include <KLocalizedString>

TextResultItem::TextResultItem(QGraphicsObject* parent, Cantor::Result* result)
    : WorksheetTextItem(parent), ResultItem(result)
{
    connect(this, SIGNAL(collapseActionSizeChanged()), parent, SLOT(recalculateSize()));
    setTextInteractionFlags(Qt::TextSelectableByMouse);
    update();

    // So useful behaviour:
    // If we have HtmlResult, but after setting we have empty document
    // So show Plain version - it more useful
    // We do it here, because we need it one
    if (document()->characterCount() && document()->characterAt(0) == QChar::ParagraphSeparator)
    {
        Cantor::HtmlResult* hr = static_cast<Cantor::HtmlResult*>(m_result);
        hr->setFormat(Cantor::HtmlResult::PlainAlternative);
        setHtml(hr->toHtml());
    }
}

double TextResultItem::setGeometry(double x, double y, double w)
{
    WorksheetTextItem::setGeometry(x, y, w);
    collapseExtraLines();
    return height();
}

void TextResultItem::populateMenu(QMenu* menu, QPointF pos)
{
    QAction * copy = KStandardAction::copy(this, SLOT(copy()), menu);
    if (!textCursor().hasSelection())
        copy->setEnabled(false);
    menu->addAction(copy);
    ResultItem::addCommonActions(this, menu);

    Cantor::Result* res = result();
    if (res->type() == Cantor::LatexResult::Type) {
        QAction* showCodeAction = nullptr;
        Cantor::LatexResult* lres = static_cast<Cantor::LatexResult*>(res);
        if (lres->isCodeShown())
            showCodeAction = menu->addAction(i18n("Show Rendered"));
        else
            showCodeAction = menu->addAction(i18n("Show Code"));

        connect(showCodeAction, &QAction::triggered, this, &TextResultItem::toggleLatexCode);
    } else if (res->type() == Cantor::HtmlResult::Type) {
        Cantor::HtmlResult* hres = static_cast<Cantor::HtmlResult*>(res);
        switch (hres->format())
        {
            case Cantor::HtmlResult::Html:
                connect(menu->addAction(i18n("Show HTML Code")), &QAction::triggered, this, &TextResultItem::showHtmlSource);
                if (!hres->plain().isEmpty())
                    connect(menu->addAction(i18n("Show Plain Alternative")), &QAction::triggered, this, &TextResultItem::showPlain);
                break;

            case Cantor::HtmlResult::HtmlSource:
                connect(menu->addAction(i18n("Show Html")), &QAction::triggered, this, &TextResultItem::showHtml);
                if (!hres->plain().isEmpty())
                    connect(menu->addAction(i18n("Show Plain Alternative")), &QAction::triggered, this, &TextResultItem::showPlain);
                break;

            case Cantor::HtmlResult::PlainAlternative:
                connect(menu->addAction(i18n("Show HTML")), &QAction::triggered, this, &TextResultItem::showHtml);
                connect(menu->addAction(i18n("Show HTML Code")), &QAction::triggered, this, &TextResultItem::showHtmlSource);
                break;

        }
    }

    menu->addSeparator();
    qDebug() << "populate Menu";
    emit menuCreated(menu, mapToParent(pos));
}

void TextResultItem::update()
{
    Q_ASSERT(
        m_result->type() == Cantor::TextResult::Type
        || m_result->type() == Cantor::LatexResult::Type
        || m_result->type() == Cantor::MimeResult::Type
        || m_result->type() == Cantor::HtmlResult::Type
    );
    switch(m_result->type()) {
    case Cantor::TextResult::Type:
    case Cantor::MimeResult::Type:
    case Cantor::HtmlResult::Type:
        setHtml(m_result->toHtml());
        break;
    case Cantor::LatexResult::Type:
        setLatex(static_cast<Cantor::LatexResult*>(m_result));
        break;
    default:
        break;
    }
}

void TextResultItem::setLatex(Cantor::LatexResult* result)
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    QString latex = result->toLatex().trimmed();
    if (latex.startsWith(QLatin1String("\\begin{eqnarray*}")) &&
        latex.endsWith(QLatin1String("\\end{eqnarray*}"))) {
        latex = latex.mid(17);
        latex = latex.left(latex.size() - 15);
    }

#ifdef WITH_EPS
    if (result->isCodeShown()) {
        if (latex.isEmpty())
            cursor.removeSelectedText();
        else
            cursor.insertText(latex);
    } else {
        QTextImageFormat format;

        if (!result->image().isNull() && worksheet()->renderer()->scale() == 1.0)
        {
            cursor.insertText(QString(QChar::ObjectReplacementCharacter), toFormat(result->image(), latex));
        }
        else
        {
            QString uuid = Cantor::LatexRenderer::genUuid();
            Cantor::Renderer* renderer = qobject_cast<Worksheet*>(scene())->renderer();;
            format = renderer->render(cursor.document(), Cantor::Renderer::EPS, result->url(), uuid);
            format.setProperty(Cantor::Renderer::CantorFormula,
                            Cantor::Renderer::LatexFormula);
            format.setProperty(Cantor::Renderer::Code, latex);
            format.setProperty(Cantor::Renderer::Delimiter, QLatin1String("$$"));
            if(format.isValid())
                cursor.insertText(QString(QChar::ObjectReplacementCharacter), format);
            else
                cursor.insertText(i18n("Cannot render Eps file. You may need additional packages"));
        }
    }
#else
    cursor.insertText(QString(QChar::ObjectReplacementCharacter), toFormat(result->image(), latex));
#endif
}

double TextResultItem::width() const
{
    return WorksheetTextItem::width();
}

double TextResultItem::height() const
{
    return WorksheetTextItem::height();
}

void TextResultItem::toggleLatexCode()
{
     Cantor::LatexResult* lr = static_cast<Cantor::LatexResult*>(result());
     if(lr->isCodeShown())
         lr->showRendered();
     else
         lr->showCode();

     parentEntry()->updateEntry();
}

void TextResultItem::showHtml()
{
     Cantor::HtmlResult* hr = static_cast<Cantor::HtmlResult*>(result());
     hr->setFormat(Cantor::HtmlResult::Html);
     parentEntry()->updateEntry();
}

void TextResultItem::showHtmlSource()
{
     Cantor::HtmlResult* hr = static_cast<Cantor::HtmlResult*>(result());
     hr->setFormat(Cantor::HtmlResult::HtmlSource);
     parentEntry()->updateEntry();
}

void TextResultItem::showPlain()
{
     Cantor::HtmlResult* hr = static_cast<Cantor::HtmlResult*>(result());
     hr->setFormat(Cantor::HtmlResult::PlainAlternative);
     parentEntry()->updateEntry();
}

void TextResultItem::saveResult()
{
    Cantor::Result* res = result();
    const QString& filename = QFileDialog::getSaveFileName(worksheet()->worksheetView(), i18n("Save result"), QString(), res->mimeType());
    qDebug() << "saving result to " << filename;
    res->save(filename);
}

void TextResultItem::deleteLater()
{
    WorksheetTextItem::deleteLater();
}

QTextImageFormat TextResultItem::toFormat(const QImage& image, const QString& latex)
{
    QTextImageFormat format;

    QUrl internal;
    internal.setScheme(QLatin1String("internal"));
    internal.setPath(Cantor::LatexRenderer::genUuid());

    document()->addResource(QTextDocument::ImageResource, internal, QVariant(image) );

    format.setName(internal.url());
    format.setProperty(Cantor::Renderer::CantorFormula, Cantor::Renderer::LatexFormula);
    //format.setProperty(Cantor::EpsRenderer::ImagePath, filename);
    format.setProperty(Cantor::Renderer::Code, latex);
    format.setProperty(Cantor::Renderer::Delimiter, QLatin1String("$$"));

    return format;
}

void TextResultItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    if (!m_isCollapsed)
        return;

    m_userCollapseOverride = !m_userCollapseOverride;
    if (m_isCollapsed)
    {
        if (m_userCollapseOverride)
        {
            update();
        }
        else
        {
            m_isCollapsed = false;
            collapseExtraLines();
        }
        emit collapseActionSizeChanged();
    }
    QGraphicsTextItem::mouseDoubleClickEvent(event);
}

int TextResultItem::visibleLineCount()
{
    int lineCounter = 0;
    QTextCursor cursor(document());
    if(!cursor.isNull())
    {
        cursor.movePosition(QTextCursor::Start);
        bool isNotDone = true;
        while( isNotDone )
        {
            isNotDone = cursor.movePosition( QTextCursor::Down );
            lineCounter++;
        }
    }

    return lineCounter;
}

void TextResultItem::collapseExtraLines()
{
    if (m_userCollapseOverride)
        return;

    int limit = Settings::visibleLinesLimit();

    // If limit disable (0 is for unlimited mode), then exit
    if (limit == 0)
        return;

    // for situation, when we have collapsed text result and resized Cantor window
    if (m_isCollapsed && (int)width() != m_widthWhenCollapsed)
    {
        update();
        m_isCollapsed = false;
    }

    if (visibleLineCount() > limit)
    {
        QTextCursor cursor(document());
        cursor.movePosition(QTextCursor::Start);
        if (limit > 4)
        {
            for (int i = 0; i < limit-4; i++)
                cursor.movePosition(QTextCursor::Down);

            cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
            cursor.movePosition(QTextCursor::Up, QTextCursor::KeepAnchor);

            cursor.insertText(QLatin1String("\n\n...\n\n"));
        }
        else
        {
            for (int i = 0; i < limit-1; i++)
                cursor.movePosition(QTextCursor::Down);
            cursor.movePosition(QTextCursor::EndOfLine);

            QString replacer = QLatin1String("...");
            for (int i = 0; i < replacer.length(); i++)
                cursor.movePosition(QTextCursor::Left);

            cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
            cursor.insertText(replacer);
        }

        m_isCollapsed = true;
        m_widthWhenCollapsed = (int)width();
    }
}
