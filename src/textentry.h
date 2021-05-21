/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#ifndef TEXTENTRY_H
#define TEXTENTRY_H

#include <QString>
#include <QDomElement>
#include <QDomDocument>
#include <QIODevice>
#include <KZip>
#include <QTextCursor>
#include <KArchive>

#include "worksheetentry.h"
#include "worksheettextitem.h"
#include "mathrendertask.h"

class TextEntry : public WorksheetEntry
{
  Q_OBJECT
  public:
    explicit TextEntry(Worksheet* worksheet);
    ~TextEntry() override;

    enum {Type = UserType + 1};
    int type() const override;

    QString text() const;

    bool isEmpty() override;

    bool acceptRichText() override;

    bool focusEntry(int pos = WorksheetTextItem::TopLeft, qreal xCoord=0) override;

    // do we need/get this?
    //bool worksheetContextMenuEvent(...);
    //void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

    void setContent(const QString& content) override;
    void setContent(const QDomElement& content, const KZip& file) override;
    void setContentFromJupyter(const QJsonObject& cell) override;
    static bool isConvertableToTextEntry(const QJsonObject& cell);

    QDomElement toXml(QDomDocument& doc, KZip* archive) override;
    QJsonValue toJupyterJson() override;
    QString toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq) override;

    void interruptEvaluation() override;

    void layOutForWidth(qreal entry_zone_x, qreal w, bool force = false) override;

    int searchText(const QString& text, const QString& pattern,
                   QTextDocument::FindFlags qt_flags);
    WorksheetCursor search(const QString& pattern, unsigned flags,
                           QTextDocument::FindFlags qt_flags,
                           const WorksheetCursor& pos = WorksheetCursor()) override;

  public Q_SLOTS:
    bool evaluate(WorksheetEntry::EvaluationOption evalOp = FocusNext) override;
    void resolveImagesAtCursor();
    void updateEntry() override;
    void populateMenu(QMenu* menu, QPointF pos) override;
    void convertToRawCell();
    void convertToTextEntry();
    void convertTargetChanged(QAction* action);

  protected:
    bool wantToEvaluate() override;

  protected Q_SLOTS:
    void handleMathRender(QSharedPointer<MathRenderResult> result);

  private:
    QTextCursor findLatexCode(const QTextCursor& cursor = QTextCursor()) const;
    QString showLatexCode(QTextCursor& cursor);
    void addNewTarget(const QString& target);

  private:
    bool m_rawCell;
    QString m_convertTarget;
    QActionGroup* m_targetActionGroup;
    QAction* m_ownTarget;
    QMenu* m_targetMenu;

    WorksheetTextItem* m_textItem;
};

#endif //TEXTENTRY_H
