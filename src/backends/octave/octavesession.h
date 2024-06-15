/*
    SPDX-FileCopyrightText: 2010 Miha Čančula <miha.cancula@gmail.com>
    SPDX-FileCopyrightText: 2017-2023 by Alexander Semke (alexander.semke@web.de)

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef OCTAVESESSION_H
#define OCTAVESESSION_H

#include <session.h>
#include <QTextStream>
#include <QRegularExpression>

namespace Cantor {
class DefaultVariableModel;
}

class OctaveExpression;
class QProcess;

class OctaveSession : public Cantor::Session
{
    Q_OBJECT
    public:
        explicit OctaveSession(Cantor::Backend*);
        ~OctaveSession() override;
        void interrupt() override;
        Cantor::Expression* evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behavior = Cantor::Expression::FinishingBehavior::DoNotDelete, bool internal = false) override;
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
        QProcess* m_process{nullptr};
        QTextStream m_stream;
        QRegularExpression m_prompt;
        QRegularExpression m_subprompt;
        int m_previousPromptNumber{1};
        bool m_syntaxError{false};
        QString m_output;
        QString m_plotFilePrefixPath;
        bool m_isIntegratedPlotsEnabled{false}; // Better move it in worksheet, like isCompletion, etc.
        bool m_writableTempFolder{false};

    private:
        void readFromOctave(QByteArray);
        bool isDoNothingCommand(const QString&);
        bool isSpecialOctaveCommand(const QString&);
        void checkWritableTempFolder();

    private Q_SLOTS:
        void readOutput();
        void readError();
        void processError();
};

#endif // OCTAVESESSION_H
