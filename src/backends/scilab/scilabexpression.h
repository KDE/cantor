/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2011 Filipe Saraiva <filipe@kde.org>
*/

#ifndef _SCILABEXPRESSION_H
#define _SCILABEXPRESSION_H

#include "expression.h"
#include <QStringList>

class ScilabExpression : public Cantor::Expression
{
    Q_OBJECT

    public:
        explicit ScilabExpression(Cantor::Session* session, bool internal = false);
        ~ScilabExpression() override = default;

        void evaluate() override;
        void interrupt() override;
        void parseOutput(QString output);
        void parseError(QString error);
        void parsePlotFile(QString filename);
        void setPlotPending(bool plot);

    public Q_SLOTS:
        void evalFinished();

    private:
        QString m_output;
        bool m_finished;
        bool m_plotPending;
};

#endif /* _SCILABEXPRESSION_H */
