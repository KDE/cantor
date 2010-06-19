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
    Copyright (C) 2010 Raffaele De Feo <alberthilbert@gmail.com>
 */

#ifndef _TEXTENTRY_H
#define _TEXTENTRY_H

#include "worksheetentry.h"
#include <QObject>

class Worksheet;
class WorksheetEntry;
class KZip;

class TextEntry : public WorksheetEntry
{
  Q_OBJECT
  public:
    TextEntry(QTextCursor position, Worksheet* parent);
    ~TextEntry();

    enum {Type = 1};
    int type();

    bool isEmpty();

    QTextCursor closestValidCursor(const QTextCursor& cursor);
    QTextCursor firstValidCursorPosition();
    QTextCursor lastValidCursorPosition();
    bool isValidCursor(const QTextCursor& cursor);

    bool acceptRichText();
    bool acceptsDrop(const QTextCursor& cursor);

    void setContent(const QString& content);
    void setContent(const QDomElement& content, const KZip& file);

    QDomElement toXml(QDomDocument& doc, KZip* archive);
    QString toPlain(QString& commandSep, QString& commentStartingSeq, QString& commentEndingSeq);

    void interruptEvaluation();

    bool evaluate(bool current);
  public slots:
    void update();
};

#endif /* _TEXTENTRY_H */
