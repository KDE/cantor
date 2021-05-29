/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2020 Nikita Sirgienko <warquark@gmail.com>
*/

#ifndef HIEARARCHYENTRY_H
#define HIEARARCHYENTRY_H

#include <QString>
#include <QDomElement>
#include <QDomDocument>
#include <KZip>

#include "worksheetentry.h"
#include "worksheettextitem.h"

class QActionGroup;
class QMenu;

class HierarchyEntry : public WorksheetEntry
{
  Q_OBJECT
  public:
    // Should be plane and int: "1, 2, 3", but not "1, 2, 4"
    enum class HierarchyLevel {
        Chapter = 1,
        Subchapter = 2,
        Section = 3,
        Subsection = 4,
        Paragraph = 5,
        Subparagraph = 6,

        EndValue = 7
    };

    explicit HierarchyEntry(Worksheet* worksheet);
    ~HierarchyEntry() override;

    enum {Type = UserType + 9};
    int type() const override;

    QString text() const;
    QString hierarchyText() const;

    HierarchyLevel level() const;
    void setLevel(HierarchyLevel);
    int hierarchyNumber() const;

    void updateHierarchyLevel(std::vector<int>& currectNumbers);
    qreal hierarchyItemWidth();

    void updateControlElementForHierarchy(qreal responsibilityZoneYEnd, int maxHierarchyDepth, bool haveSubElements);

    bool isEmpty() override;

    bool acceptRichText() override;

    bool focusEntry(int pos = WorksheetTextItem::TopLeft, qreal xCoord=0) override;

    void setContent(const QString& content) override;
    void setContent(const QDomElement& content, const KZip& file) override;
    void setContentFromJupyter(const QJsonObject& cell) override;
    static bool isConvertableToHierarchyEntry(const QJsonObject& cell);

    QDomElement toXml(QDomDocument& doc, KZip* archive) override;
    QJsonValue toJupyterJson() override;
    QString toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq) override;

    void layOutForWidth(qreal entry_zone_x, qreal w, bool force = false) override;

    int searchText(const QString& text, const QString& pattern,
                   QTextDocument::FindFlags qt_flags);
    WorksheetCursor search(const QString& pattern, unsigned flags,
                           QTextDocument::FindFlags qt_flags,
                           const WorksheetCursor& pos = WorksheetCursor()) override;

    void startDrag(QPointF grabPos = QPointF()) override;

  Q_SIGNALS:
    void hierarhyEntryNameChange(QString name, QString searchName, int depth);

  public Q_SLOTS:
    bool evaluate(WorksheetEntry::EvaluationOption evalOp = FocusNext) override;
    void updateEntry() override;
    void populateMenu(QMenu* menu, QPointF pos) override;
    void handleControlElementDoubleClick();
    void updateAfterSettingsChanges() override;

  protected:
    bool wantToEvaluate() override;
    void recalculateControlGeometry() override;

  private Q_SLOTS:
    void setLevelTriggered(QAction*);

  private:
    void updateFonts(bool force = false);

  private:

    WorksheetTextItem* m_hierarchyLevelItem;
    WorksheetTextItem* m_textItem;
    HierarchyLevel m_depth;
    int m_hierarchyNumber;
    QActionGroup* m_setLevelActionGroup;
    QMenu* m_setLevelMenu;
    WorksheetEntry* m_hidedSubentries;
};

#endif // HIEARARCHYENTRY_H
