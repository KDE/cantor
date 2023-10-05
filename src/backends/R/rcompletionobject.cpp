/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2010 Oleksiy Protas <elfy.ua@gmail.com>
*/

#include "rcompletionobject.h"
#include "rkeywords.h"
#include "rsession.h"
#include "result.h"

using namespace Cantor;

RCompletionObject::RCompletionObject(const QString& command, int index, RSession* session):
    CompletionObject(session),
    m_expression(nullptr)
{
    setLine(command, index);
}

RCompletionObject::~RCompletionObject()
{
    if (m_expression)
        m_expression->setFinishingBehavior(Expression::FinishingBehavior::DeleteOnFinish);
}

void RCompletionObject::fetchCompletions()
{
    if (session()->status() != Session::Done)
    {
        QStringList allCompletions;

        allCompletions << RKeywords::instance()->keywords();

        setCompletions(allCompletions);
        Q_EMIT fetchingDone();
    }
    else
    {
        if (m_expression)
            return;

        const QString cmd = QLatin1String("%completion ")+command();
        m_expression = session()->evaluateExpression(cmd, Expression::FinishingBehavior::DoNotDelete, true);
        connect(m_expression, &Expression::statusChanged, this, &RCompletionObject::receiveCompletions);
    }
}

void RCompletionObject::receiveCompletions(Cantor::Expression::Status status)
{
    switch(status)
    {
        case Expression::Status::Done:
        {
            if (!m_expression->result())
                break;

            const QChar recordSep(30);
            const QChar unitSep(31);

            const QString output = m_expression->result()->data().toString();

            const QString& token = output.section(unitSep, 0, 0);
            const QStringList& options = output.section(unitSep, 1, 1).split(recordSep, Qt::SkipEmptyParts);

            // TODO: investigate the empty token problem
            /* Not so fast, evidently KCompletion requires a nonempty token, hence this stub */
            if (token.length()==0 && command().length()!=0)
            {
                /* Adding previous symbol to token, ugly but effective */
                QString lastchar(command().at(command().length()-1));
                setCommand(lastchar);
                setCompletions(QStringList(options).replaceInStrings(QRegExp(QLatin1String("^")), lastchar));
            }
            else
            {
                setCommand(token);
                setCompletions(options);
            }
            break;
        }
        case Expression::Status::Error:
            qWarning() << "R code for completion command finishs with error message: " << m_expression->errorMessage();
            break;

        case Expression::Status::Interrupted:
            break;

        default:
            return;
    }

    m_expression->deleteLater();
    m_expression = nullptr;
    Q_EMIT fetchingDone();
}
