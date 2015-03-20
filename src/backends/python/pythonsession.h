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
    Copyright (C) 2015 Minh Ngo <minh@fedoraproject.org>
 */

#ifndef _PYTHONSESSION_H
#define _PYTHONSESSION_H

#include "session.h"
#include <QStringList>
#include <QPointer>

namespace Cantor {
class DefaultVariableModel;
}

class PythonExpression;
class KTemporaryFile;
class KDirWatch;
class KProcess;

class CANTOR_EXPORT PythonSession : public Cantor::Session
{
  Q_OBJECT
  public:
    PythonSession(Cantor::Backend* backend);
    ~PythonSession();

    void login();
    void logout();

    void interrupt();
    void runExpression(PythonExpression* expr);

    Cantor::Expression* evaluateExpression(const QString& command, Cantor::Expression::FinishingBehavior behave);
    Cantor::CompletionObject* completionFor(const QString& command, int index=-1);
    virtual QSyntaxHighlighter* syntaxHighlighter(QObject* parent);
    virtual QAbstractItemModel* variableModel();

    virtual bool integratePlots() const = 0;
    virtual QStringList autorunScripts() const = 0;

  private:
    KDirWatch* m_watch;
    QStringList m_listPlotName;
    Cantor::DefaultVariableModel* m_variableModel;

    QList<PythonExpression*> m_runningExpressions;
    PythonExpression* m_currentExpression;

  protected:
    QString m_output;
    QString m_error;

  private:
    void listVariables();

    void getPythonCommandOutput(const QString& commandProcessing);

    QString identifyPythonModule(const QString& command) const;
    QString identifyVariableModule(const QString& command) const;
    bool identifyKeywords(const QString& command);

    virtual void runPythonCommand(const QString& command) const = 0;
    virtual QString getOutput() const = 0;
    virtual QString getError() const = 0;

    virtual void readExpressionOutput(const QString& commandProcessing);

  protected:
    void runClassOutputPython() const;
    void updateOutput();

  private Q_SLOTS:
    void readOutput(const QString& commandProcessing);
    void plotFileChanged(const QString& filename);
    void expressionFinished();

  Q_SIGNALS:
    void updateHighlighter();
};

#endif /* _PYTHONSESSION_H */
