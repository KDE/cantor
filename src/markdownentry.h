/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018 Yifei Wu <kqwyfg@gmail.com>
*/

#ifndef MARKDOWNENTRY_H
#define MARKDOWNENTRY_H

#include <vector>

#include <QSharedPointer>

#include "worksheetentry.h"
#include "worksheettextitem.h"

#include "mathrendertask.h"
#include "lib/latexrenderer.h"

class QJsonObject;

class MarkdownEntry : public WorksheetEntry
{
  Q_OBJECT
  public:
    explicit MarkdownEntry(Worksheet* worksheet);
    ~MarkdownEntry() override = default;

    enum {Type = UserType + 7};
    int type() const override;

    bool isEmpty() override;

    bool acceptRichText() override;

    bool focusEntry(int pos = WorksheetTextItem::TopLeft, qreal xCoord=0) override;

    void setContent(const QString& content) override;
    void setContent(const QDomElement& content, const KZip& file) override;
    void setContentFromJupyter(const QJsonObject& cell) override;

    QDomElement toXml(QDomDocument& doc, KZip* archive) override;
    QJsonValue toJupyterJson() override;
    QString toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq) override;

    QString plainText() const;

    void interruptEvaluation() override;

    void layOutForWidth(qreal entry_zone_x, qreal w, bool force = false) override;

    WorksheetCursor search(const QString& pattern, unsigned flags,
                           QTextDocument::FindFlags qt_flags,
                           const WorksheetCursor& pos = WorksheetCursor()) override;

  public Q_SLOTS:
    bool evaluate(WorksheetEntry::EvaluationOption evalOp = FocusNext) override;
    void updateEntry() override;
    void populateMenu(QMenu* menu, QPointF pos) override;

  protected:
    bool renderMarkdown(QString& plain);
    bool eventFilter(QObject* object, QEvent* event) override;
    bool wantToEvaluate() override;
    void setRenderedHtml(const QString& html);
    void setPlainText(const QString& plain);
    void renderMath();
    void renderMathExpression(int jobId, QString mathCode);
    void setRenderedMath(int jobId, const QTextImageFormat& format, const QUrl& internal, const QImage& image);
    QTextCursor findMath(int id);
    void markUpMath();
    void addImageAttachment(const QString& name, const QImage& image);

    static std::pair<QString, Cantor::LatexRenderer::EquationType> parseMathCode(QString mathCode);

  protected Q_SLOTS:
    void handleMathRender(QSharedPointer<MathRenderResult> result);
    void insertImage();
    void clearAttachments();
    void enterEditMode();

  protected:
    WorksheetTextItem* m_textItem;
    QString plain;
    QString html;
    bool rendered;
    std::vector<std::pair<QUrl,QString>> attachedImages;
    std::vector<std::pair<QString, bool>> foundMath;
    static const int JobProperty = 10000;
};

#endif //MARKDOWNENTRY_H
