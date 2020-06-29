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
#include <QRegularExpression>
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
        explicit OctaveSession(Cantor::Backend* backend);
        ~OctaveSession() override;
        void interrupt() override;
        Cantor::Expression* evaluateExpression(const QString& command, Cantor::Expression::FinishingBehavior finishingBehavior = Cantor::Expression::FinishingBehavior::DoNotDelete, bool internal = false) override;
        void logout() override;
        void login() override;
        Cantor::CompletionObject* completionFor(const QString& cmd, int index=-1) override;
        Cantor::SyntaxHelpObject* syntaxHelpFor(const QString& cmd) override;
        QSyntaxHighlighter* syntaxHighlighter(QObject* parent) override;
        void runFirstExpression() override;

        bool isIntegratedPlotsEnabled() const;
        QString plotFilePrefixPath() const;

    private:
        const static QRegularExpression PROMPT_UNCHANGEABLE_COMMAND;

    private:
        KProcess* m_process;
        QTextStream m_stream;
        QRegularExpression m_prompt;
        QRegularExpression m_subprompt;
        int m_previousPromptNumber;

        bool m_syntaxError;

        QString m_output;
        QString m_plotFilePrefixPath;
        bool m_isIntegratedPlotsEnabled; // Better move it in worksheet, like isCompletion, etc.
        bool m_isIntegratedPlotsSettingsEnabled;

    private:
        void readFromOctave(QByteArray data);
        bool isDoNothingCommand(const QString& command);
        bool isSpecialOctaveCommand(const QString& command);
        void updateGraphicPackagesFromSettings();

    private Q_SLOTS:
        void readOutput();
        void readError();
        void currentExpressionStatusChanged(Cantor::Expression::Status status);
        void processError();
        void runSpecificCommands();
};

#endif // OCTAVESESSION_H
