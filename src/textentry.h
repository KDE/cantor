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

#ifndef TEXTENTRY_H
#define TEXTENTRY_H

#include <qstring.h>
#include <qdomelement.h>
#include <qdomdocument.h>
#include <kzip>

#include "worksheetentry.h"
#include "worksheettextitem.h"

class TextEntry : public WorksheetEntry
{
  Q_OBJECT
  public:
    TextEntry();
    ~TextEntry();

    enum {Type = UserType + 1};
    int type() const;

    bool isEmpty();

    bool acceptRichText();

    // do we need/get this?
    //bool worksheetContextMenuEvent(...);
    vool mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

    void setContent(const QString& content);
    void setContent(const QDomElement& content, const KZip& file);

    QDomElement toXml(QDomDocument& doc, KZip* archive);
    QString toPlain(QString& commandSep, QString& commentStartingSeq, QString& commentEndingSeq);

    void interruptEvaluation();

    bool evaluate(bool current);

  public slots:
    void updateEntry();

  private:
    QTextCursor findLatexCode(QTextDocument *doc) const;
    void showLatexCode(QTextCursor cursor);

  private:
    WorksheetTextItem* m_textItem;

};

#endif TEXTENTRY_H
