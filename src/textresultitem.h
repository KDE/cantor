/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#ifndef TEXTRESULTITEM_H
#define TEXTRESULTITEM_H

#include "resultitem.h"
#include "worksheettextitem.h"

namespace Cantor {
    class LatexResult;
}

class TextResultItem : public WorksheetTextItem, public ResultItem
{
  Q_OBJECT
  public:
    explicit TextResultItem(WorksheetEntry*, Cantor::Result*);
    ~TextResultItem() override = default;

    using WorksheetTextItem::setGeometry;
    double setGeometry(double x, double y, double w) override;
    void populateMenu(QMenu*, QPointF) override;

    void update() override;

    void setLatex(Cantor::LatexResult*);
    QTextImageFormat toFormat(const QImage&, const QString& latex);

    double width() const override;
    double height() const override;

    void deleteLater() override;

  Q_SIGNALS:
    void collapseActionSizeChanged();

  protected Q_SLOTS:
    void toggleLatexCode();
    void showHtml();
    void showHtmlSource();
    void showPlain();
    void saveResult();

  protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent*) override;
    int visibleLineCount();
    void collapseExtraLines();

  protected:
    bool m_isCollapsed{false};
    bool m_userCollapseOverride{false};
    int m_widthWhenCollapsed{0};
};

#endif //TEXTRESULTITEM_H
