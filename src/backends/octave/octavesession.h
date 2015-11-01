/*
    Copyright (C) 2010 Miha Čančula <miha.cancula@gmail.com>

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
*/

#ifndef OCTAVESESSION_H
#define OCTAVESESSION_H

#include <session.h>
#include <QQueue>
#include <QTextStream>
#include <QRegExp>
#include <QPointer>

namespace Cantor {
class DefaultVariableModel;
}

class KDirWatch;
class OctaveExpression;
class KProcess;


class OctaveSession : public Cantor::Session
{
    Q_OBJECT
    public:
    OctaveSession(Cantor::Backend* backend);
    ~OctaveSession();
    virtual void interrupt();
    virtual Cantor::Expression* evaluateExpression(const QString& command, Cantor::Expression::FinishingBehavior finishingBehavior);
    virtual void logout();
    virtual void login();
    virtual Cantor::CompletionObject* completionFor(const QString& cmd, int index=-1);
    virtual Cantor::SyntaxHelpObject* syntaxHelpFor(const QString& cmd);
    virtual QSyntaxHighlighter* syntaxHighlighter(QObject* parent);
    virtual QAbstractItemModel* variableModel();

    void runExpression(OctaveExpression* expression);

    private:
        KProcess* m_process;
        QTextStream m_stream;
        QQueue <OctaveExpression*> m_expressionQueue;
        QPointer <OctaveExpression> m_currentExpression;
        QRegExp m_prompt;

        KDirWatch* m_watch;
        QString m_tempDir;

        Cantor::DefaultVariableModel* m_variableModel;

        void readFromOctave(QByteArray data);

    private Q_SLOTS:
        void readOutput();
        void readError();
        void currentExpressionStatusChanged(Cantor::Expression::Status status);
        void processError();
    void plotFileChanged(const QString& filename);
    void runSpecificCommands();

  Q_SIGNALS:
    void functionsChanged();
    void variablesChanged();
};

#endif // OCTAVESESSION_H
