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

namespace Cantor {
class DefaultVariableModel;
}

class PythonExpression;
class KDirWatch;
class QDBusInterface;
class KProcess;

class CANTOR_EXPORT PythonSession : public Cantor::Session
{
  Q_OBJECT
  public:
    PythonSession(Cantor::Backend* backend, const QString serverName, const QString DbusChannelName);
    ~PythonSession() override;

    void login() override;
    void logout() override;

    void interrupt() override;
    void runExpression(PythonExpression* expr);

    Cantor::Expression* evaluateExpression(const QString& command, Cantor::Expression::FinishingBehavior behave) override;
    Cantor::CompletionObject* completionFor(const QString& command, int index=-1) override;
    QSyntaxHighlighter* syntaxHighlighter(QObject* parent) override;
    QAbstractItemModel* variableModel() override;

    virtual bool integratePlots() const = 0;
    virtual QStringList autorunScripts() const = 0;

  private:
    KDirWatch* m_watch;
    QStringList m_listPlotName;
    Cantor::DefaultVariableModel* m_variableModel;

    QList<PythonExpression*> m_runningExpressions;
    PythonExpression* m_currentExpression;

    QDBusInterface* m_pIface;
    KProcess* m_pProcess;
    QString serverName;
    QString DbusChannelName;

  protected:
    QString m_output;
    QString m_error;

  private:
    void listVariables();

    void getPythonCommandOutput(const QString& commandProcessing);

    QString identifyPythonModule(const QString& command) const;
    QString identifyVariableModule(const QString& command) const;
    bool identifyKeywords(const QString& command);

    void runPythonCommand(const QString& command) const;
    QString getOutput() const;
    QString getError() const;

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
    void newVariable(const QString variable);
};

#endif /* _PYTHONSESSION_H */
