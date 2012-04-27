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

#ifndef WORKSHEETENTRY_H
#define WORKSHEETENTRY_H

#include <graphicswidget.h>

#include "worksheet.h"

class TextEntry;
class CommandEntry;
class ImageEntry;
class PageBreakEntry;
class LaTeXEntry;

class WorksheetEntry : public QGraphicsWidget
{
  Q_OBJECT
  private:
    WorksheetEntry();
    ~WorksheetEntry();

    enum {Type = UserType};

    virtual int type() const;

    static WorksheetEntry* create(int t);

    virtual bool acceptRichText() = 0;

    virtual void setContent(const QString& content)=0;
    virtual void setContent(const QDomElement& content, const KZip& file)=0;

    virtual QDomElement toXml(QDomDocument& doc, KZip* archive)=0;
    virtual QString toPlain(QString& commandSep, QString& commentStartingSeq, QString& commentEndingSeq)=0;

    virtual void interruptEvaluation()=0;

    virtual bool evaluate(bool current)=0;

    virtual void enableHighlighting(bool highlight)=0;

  protected:
    Worksheet* worksheet();
};

#endif // WORKSHEETENTRY_H
