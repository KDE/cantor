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
    Copyright (C) 2011 Alexander Rieder <alexanderrieder@gmail.com>
 */

#ifndef _FORMULATEXTOBJECT_H
#define _FORMULATEXTOBJECT_H


#include <QTextObjectInterface>

#include "cantor_export.h"

class QTextDocument;
class QTextFormat;
class QPainter;
class QRectF;
class QSizeF;

class CANTOR_EXPORT FormulaTextObject : public QObject, public QTextObjectInterface
{
    Q_OBJECT
    Q_INTERFACES(QTextObjectInterface)
	
  public:
    enum { FormulaTextFormat = QTextFormat::UserObject + 1 };
    enum FormulaProperties { LatexCode = 1, FormulaType = 2, Data = 3, ResourceUrl = 4 };
    enum FormulaType { LatexFormula=0, MmlFormula=1 };
    

    QSizeF intrinsicSize(QTextDocument *doc, int posInDocument,
                        const QTextFormat &format);
    void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc,
                    int posInDocument, const QTextFormat &format);

};

#endif /* _FORMULATEXTOBJECT_H */
