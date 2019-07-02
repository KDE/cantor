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
    Copyright (C) 2019 Sirgienko Nikita <warquark@gmail.com>
*/

#include <QObject>

class Worksheet;
class WorksheetEntry;
namespace Cantor {
    class Expression;
}

class WorksheetTest: public QObject
{
    Q_OBJECT

  private Q_SLOTS:
    void initTestCase();
    void testJupyter1();
    void testJupyter2();
    void testJupyter3();
    void testJupyter4();

  private:
    static Worksheet* loadWorksheet(const QString& name);
    static int entriesCount(Worksheet* worksheet);
    static Cantor::Expression* expression(WorksheetEntry* entry);
    static QString plainMarkdown(WorksheetEntry* markdownEntry);
    static QString plainCommand(WorksheetEntry* commandEntry);
    static void testMarkdown(WorksheetEntry* &entry, const QString& content);
    static void testCommandEntry(WorksheetEntry* &entry, int id, const QString& content);
    static void testCommandEntry(WorksheetEntry* entry, int id, int resultsCount, const QString& content);
    static void testImageResult(WorksheetEntry* entry, int index);
    static void testTextResult(WorksheetEntry* entry, int index, const QString& content);
    static void testHTMLTextResult(WorksheetEntry* entry, int index, const QString& content);
    static void testHTMLTextResult(WorksheetEntry* entry, int index, const QString& plain, const QString& html);
};
