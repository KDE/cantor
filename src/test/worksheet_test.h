/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2019 Sirgienko Nikita <warquark@gmail.com>
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
    /* Jupyter load tests */
    void initTestCase();
    void testJupyter1();
    void testJupyter2();
    void testJupyter3();
    void testJupyter4();
    void testJupyter5();
    void testJupyter6();
    void testJupyter7();

    void testMarkdownAttachment();
    void testEntryLoad1();
    void testEntryLoad2();
    void testResultsLoad();
    void testMimeResult();
    void testMimeResultWithPlain();

    void testCommandEntryExecutionAction1();
    void testCommandEntryExecutionAction2();
    void testCollapsingAllResultsAction();
    void testRemovingAllResultsAction();

    /* common features tests */
    void testMathRender();
    void testMathRender2();

  private:
    void waitForSignal( QObject* sender, const char* signal);
    static Worksheet* loadWorksheet(const QString& name);
    static int entriesCount(Worksheet* worksheet);
    static Cantor::Expression* expression(WorksheetEntry* entry);
    static QString plainMarkdown(WorksheetEntry* markdownEntry);
    static QString plainText(WorksheetEntry* textEntry);
    static QString plainCommand(WorksheetEntry* commandEntry);
    static QString plainLatex(WorksheetEntry* latexEntry);
    static void testMarkdown(WorksheetEntry* &entry, const QString& content);
    static void testCommandEntry(WorksheetEntry* &entry, int id, const QString& content);
    static void testCommandEntry(WorksheetEntry* entry, int id, int resultsCount, const QString& content);
    static void testLatexEntry(WorksheetEntry* &entry, const QString& content);
    static void testTextEntry(WorksheetEntry* &entry, const QString& content);
    static void testImageResult(WorksheetEntry* entry, int index);
    static void testTextResult(WorksheetEntry* entry, int index, const QString& content);
    static void testHtmlResult(WorksheetEntry* entry, int index, const QString& content);
    static void testHtmlResult(WorksheetEntry* entry, int index, const QString& plain, const QString& html);
};
