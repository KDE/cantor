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
        void setWorksheetPath(const QString&) override;

        bool isIntegratedPlotsEnabled() const;
        QString plotFilePrefixPath() const;

    private:
        const static QRegularExpression PROMPT_UNCHANGEABLE_COMMAND;

    private:
        KProcess* m_process{nullptr};
        QTextStream m_stream;
        QRegularExpression m_prompt;
        QRegularExpression m_subprompt;
        int m_previousPromptNumber{1};
        bool m_syntaxError{false};
        QString m_output;
        QString m_plotFilePrefixPath;
        QString m_worksheetPath;
        bool m_isIntegratedPlotsEnabled{false}; // Better move it in worksheet, like isCompletion, etc.
        bool m_isIntegratedPlotsSettingsEnabled{false};

    private:
        void readFromOctave(QByteArray);
        bool isDoNothingCommand(const QString&);
        bool isSpecialOctaveCommand(const QString&);
        void updateGraphicPackagesFromSettings();
        QString graphicPackageErrorMessage(QString packageId) const override;

    private Q_SLOTS:
        void readOutput();
        void readError();
        void currentExpressionStatusChanged(Cantor::Expression::Status);
        void processError();
        void runSpecificCommands();
};

#endif // OCTAVESESSION_H
