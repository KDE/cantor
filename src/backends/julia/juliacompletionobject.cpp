/*
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    as published by the Free Software Foundation; either version 2
 *    of the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA  02110-1301, USA.
 *
 *    ---
 *    Copyright (C) 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
 */

#include "juliacompletionobject.h"

#include "juliasession.h"
#include "juliakeywords.h"

#include "result.h"

#include <QDebug>
#include <julia_version.h>

JuliaCompletionObject::JuliaCompletionObject(const QString &command, int index, JuliaSession *session):
    Cantor::CompletionObject(session),
    m_expression(nullptr)
{
    setLine(command, index);
}

JuliaCompletionObject::~JuliaCompletionObject()
{
    if (m_expression)
        m_expression->setFinishingBehavior(Cantor::Expression::FinishingBehavior::DeleteOnFinish);
}

void JuliaCompletionObject::fetchCompletions()
{
    if (session()->status() != Cantor::Session::Done)
    {
        QStringList allCompletions;

        allCompletions << JuliaKeywords::instance()->keywords();
        allCompletions << JuliaKeywords::instance()->variables();
        allCompletions << JuliaKeywords::instance()->functions();
        allCompletions << JuliaKeywords::instance()->plotShowingCommands();

        setCompletions(allCompletions);
        emit fetchingDone();
    }
    else
    {
        if (m_expression)
            return;

        QString completionCommand;
#if QT_VERSION_CHECK(JULIA_VERSION_MAJOR, JULIA_VERSION_MINOR, 0) >= QT_VERSION_CHECK(0, 7, 0)
        completionCommand =
            QString::fromLatin1(
                "using REPL; "
                "join("
                "map(REPL.REPLCompletions.completion_text, REPL.REPLCompletions.completions(\"%1\", %2)[1]),"
                "\"__CANTOR_DELIM__\")"
            );
#else
        completionCommand =
            QString::fromLatin1(
                "join("
                "Base.REPL.REPLCompletions.completions(\"%1\", %2)[1],"
                "\"__CANTOR_DELIM__\")"
            );
#endif
        const QString cmd = completionCommand.arg(command()).arg(command().size());
        m_expression = session()->evaluateExpression(cmd, Cantor::Expression::FinishingBehavior::DoNotDelete, true);
        connect(m_expression, &Cantor::Expression::statusChanged, this, &JuliaCompletionObject::extractCompletions);
    }
}

void JuliaCompletionObject::extractCompletions(Cantor::Expression::Status status)
{
    switch(status)
    {
        case Cantor::Expression::Done:
        {
            auto result = m_expression->result()->data().toString();
            result.chop(1);
            result.remove(0, 1);
            QStringList completions = result.split(QLatin1String("__CANTOR_DELIM__"));
            if (command().contains(QLatin1Char('.')))
                for(QString& word : completions)
                {
                    const int i = command().lastIndexOf(QLatin1Char('.'));
                    const QString& prefix = command().remove(i, command().size()-i) + QLatin1Char('.');
                    if (!word.startsWith(prefix))
                        word.prepend(prefix);
                }

            setCompletions(completions);
            break;
        }
        case Cantor::Expression::Interrupted:
        case Cantor::Expression::Error:
        {
            qDebug() << "fetching expression finished with status" << (status == Cantor::Expression::Error? "Error" : "Interrupted");

            break;
        }
        default:
            return;
    }

    m_expression->deleteLater();
    m_expression = nullptr;
    emit fetchingDone();
}

bool JuliaCompletionObject::mayIdentifierContain(QChar c) const
{
    return c.isLetter() || c.isDigit() || c == QLatin1Char('_') ||
        c == QLatin1Char('%') || c == QLatin1Char('$') || c == QLatin1Char('.');
}

bool JuliaCompletionObject::mayIdentifierBeginWith(QChar c) const
{
    return c.isLetter() || c == QLatin1Char('_') || c == QLatin1Char('%') ||
        c == QLatin1Char('$');
}
