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
    Copyright (C) 2012 Martin Kuettler <martin.kuettler@gmail.com>
 */

#ifndef TEXTRESULTITEM_H
#define TEXTRESULTITEM_H

#include "resultitem.h"
#include "worksheettextitem.h"
#include "worksheetentry.h"
#include "epsrenderer.h"

#include "lib/latexresult.h"

class TextResultItem : public WorksheetTextItem, public ResultItem
{
  Q_OBJECT
  public:
    TextResultItem(QGraphicsObject* parent);
    ~TextResultItem() override;

    using WorksheetTextItem::setGeometry;
    double setGeometry(double x, double y, double w) override;
    void populateMenu(QMenu* menu, QPointF pos) override;

    ResultItem* updateFromResult(Cantor::Result* result) override;

    void setLatex(Cantor::LatexResult* result);

    double width() const override;
    double height() const override;

    void deleteLater() override;
    EpsRenderer* epsRenderer();
    CommandEntry* parentEntry();
    Cantor::Result* result() override;

  Q_SIGNALS:
    void removeResult();

  protected Q_SLOTS:
    void toggleLatexCode();
    void saveResult();
};

#endif //TEXTRESULTITEM_H
