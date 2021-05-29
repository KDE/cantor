/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2011 Filipe Saraiva <filipe@kde.org>
*/

#ifndef _SCILABSESSION_H
#define _SCILABSESSION_H

#include "session.h"
#include "scilabexpression.h"
#include <QStringList>
#include <QQueue>

namespace Cantor {
    class DefaultVariableModel;
}

class ScilabExpression;
class KDirWatch;
class QProcess;

class ScilabSession : public Cantor::Session
{
    Q_OBJECT

    public:
        explicit ScilabSession(Cantor::Backend* backend);
        ~ScilabSession() override;

        void login() override;
        void logout() override;

        void interrupt() override;
        void runExpression(ScilabExpression* expr);

        QSyntaxHighlighter* syntaxHighlighter(QObject* parent) override;

        Cantor::Expression* evaluateExpression(const QString& command, Cantor::Expression::FinishingBehavior behave = Cantor::Expression::FinishingBehavior::DoNotDelete, bool internal = false) override;
        Cantor::CompletionObject* completionFor(const QString& command, int index=-1) override;
        void runFirstExpression() override;
        Cantor::DefaultVariableModel* variableModel() const override;

    public Q_SLOTS:
        void readOutput();
        void readError();
        void plotFileChanged(const QString& filename);

    private:
        QProcess* m_process;
        KDirWatch* m_watch;
        QStringList m_listPlotName;
        QString m_output;
        Cantor::DefaultVariableModel* m_variableModel;
    private Q_SLOTS:
        void currentExpressionStatusChanged(Cantor::Expression::Status status);

};

#endif /* _SCILABSESSION_H */
