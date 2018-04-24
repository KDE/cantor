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
    Copyright (C) 2011 Filipe Saraiva <filipe@kde.org>
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
        ScilabSession(Cantor::Backend* backend);
        ~ScilabSession() override;

        void login() Q_DECL_OVERRIDE;
        void logout() Q_DECL_OVERRIDE;

        void interrupt() Q_DECL_OVERRIDE;

        QSyntaxHighlighter* syntaxHighlighter(QObject* parent) Q_DECL_OVERRIDE;

        Cantor::Expression* evaluateExpression(const QString& command, Cantor::Expression::FinishingBehavior behave) Q_DECL_OVERRIDE;
        void runFirstExpression() Q_DECL_OVERRIDE;
        Cantor::CompletionObject* completionFor(const QString& command, int index=-1) Q_DECL_OVERRIDE;
        QAbstractItemModel* variableModel() Q_DECL_OVERRIDE;

    public Q_SLOTS:
        void readOutput();
        void readError();
        void plotFileChanged(const QString& filename);

    Q_SIGNALS:
        void updateHighlighter();
        void updateVariableHighlighter();

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
