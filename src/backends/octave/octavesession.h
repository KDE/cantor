/*
    SPDX-FileCopyrightText: 2010 Miha Čančula <miha.cancula@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
        void setWorksheetPath(const QString& path) override;

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
        QString m_worksheetPath;
        bool m_isIntegratedPlotsEnabled; // Better move it in worksheet, like isCompletion, etc.
        bool m_isIntegratedPlotsSettingsEnabled;

    private:
        void readFromOctave(QByteArray data);
        bool isDoNothingCommand(const QString& command);
        bool isSpecialOctaveCommand(const QString& command);
        void updateGraphicPackagesFromSettings();
        QString graphicPackageErrorMessage(QString packageId) const override;

    private Q_SLOTS:
        void readOutput();
        void readError();
        void currentExpressionStatusChanged(Cantor::Expression::Status status);
        void processError();
        void runSpecificCommands();
};

#endif // OCTAVESESSION_H
