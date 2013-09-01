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
    Copyright (C) 2012 Filipe Saraiva <filipe@kde.org>
 */

#ifndef _PYTHONSESSION_H
#define _PYTHONSESSION_H

#include "session.h"
#include <QStringList>

#include <Python.h>

class PythonExpression;
class KTemporaryFile;
class KDirWatch;
class KProcess;

class PythonSession : public Cantor::Session
{
  Q_OBJECT
  public:
    PythonSession( Cantor::Backend* backend);
    ~PythonSession();

    void login();
    void logout();

    void interrupt();
    void runExpression(PythonExpression* expr);

    QSyntaxHighlighter* syntaxHighlighter(QObject* parent);

    Cantor::Expression* evaluateExpression(const QString& command, Cantor::Expression::FinishingBehavior behave);
    Cantor::CompletionObject* completionFor(const QString& command, int index=-1);

  public slots:
    void readOutput(PythonExpression* expr, QString commandProcessing);
//     void plotFileChanged(QString filename);

  private:
    KDirWatch* m_watch;
    QStringList m_listPlotName;
    QString m_output;
    QString m_error;

    PyObject *m_pModule;

    QList<PythonExpression*> m_runningExpressions;
    PythonExpression* m_currentExpression;

    void runClassOutputPython();

    void getPythonCommandOutput(QString commandProcessing);

    QString identifyPythonModule(QString command);
    QString identifyVariableModule(QString command);
    bool identifyKeywords(QString command);

  private slots:
    void expressionFinished();
    void currentExpressionStatusChanged(Cantor::Expression::Status status);
};

#endif /* _PYTHONSESSION_H */
