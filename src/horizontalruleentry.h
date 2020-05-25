/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2020 Sirgienko Nikita <warquark@gmail.com>
 */

#ifndef HORIZONTALLINEENTRY_H
#define HORIZONTALLINEENTRY_H

#include "worksheetentry.h"

class HorizontalRuleEntry : public WorksheetEntry
{
  public:
    enum LineType {Thin = 0, Medium = 1, Thick = 2, Count = 3};

    HorizontalRuleEntry(Worksheet* worksheet);
    ~HorizontalRuleEntry() override;

    enum {Type = UserType + 8};
    int type() const override;

    void setLineType(LineType type);
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    static bool isConvertableToHorizontalRuleEntry(const QJsonObject& cell);

    bool isEmpty() override;
    bool acceptRichText() override;
    void setContent(const QString&) override;
    void setContent(const QDomElement&, const KZip&) override;
    void setContentFromJupyter(const QJsonObject & cell) override;
    QDomElement toXml(QDomDocument&, KZip*) override;
    QJsonValue toJupyterJson() override;
    QString toPlain(const QString&, const QString&, const QString&) override;
    void interruptEvaluation() override;
    void layOutForWidth(qreal entry_zone_x, qreal w, bool force = false) override;
    void populateMenu(QMenu* menu, QPointF pos) override;

  public Q_SLOTS:
    bool evaluate(WorksheetEntry::EvaluationOption evalOp = FocusNext) override;
    void updateEntry() override;

    void changeSize(QSizeF s);

  protected:
    bool wantToEvaluate() override;

  private:
    static int lineWidth(LineType type);
    void initMenus();

  private Q_SLOTS:
    void lineTypeChanged(QAction* action);
    void lineColorChanged(QAction* action);
    void lineStyleChanged(QAction* action);

  public:
    static const qreal LineVerticalMargin;
    static constexpr unsigned int styleCount = 5;
    static const QString styleNames[styleCount];
    static const Qt::PenStyle styles[styleCount];

  private:
    LineType m_type;
    QColor m_color;
    qreal m_entry_zone_offset_x;
    qreal m_width;
    Qt::PenStyle m_style;

    bool m_menusInitialized;

    QActionGroup* m_lineTypeActionGroup;
    QMenu* m_lineTypeMenu;
    bool m_lineColorCustom;

    QActionGroup* m_lineColorActionGroup;
    QMenu* m_lineColorMenu;

    QActionGroup* m_lineStyleActionGroup;
    QMenu* m_lineStyleMenu;
};

#endif //HORIZONTALLINEENTRY_H
