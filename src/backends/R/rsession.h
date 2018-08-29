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
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
 */

#ifndef _RSESSION_H
#define _RSESSION_H

#include <QRegExp>
#include <QStringList>

#include "session.h"
#include "rserver_interface.h"

class RExpression;
class QProcess;

namespace Cantor {
class DefaultVariableModel;
}

class RSession : public Cantor::Session
{
  Q_OBJECT
  public:
    RSession( Cantor::Backend* backend);
    ~RSession() override;

    void login() override;
    void logout() override;

    void interrupt() override;

    Cantor::Expression* evaluateExpression(const QString& command, Cantor::Expression::FinishingBehavior behave = Cantor::Expression::FinishingBehavior::DoNotDelete, bool internal = false) override;
    Cantor::CompletionObject* completionFor(const QString& command, int index=-1) override;
    QSyntaxHighlighter* syntaxHighlighter(QObject* parent) override;
    QAbstractItemModel* variableModel() override;
    void sendInputToServer(const QString& input);
    void runFirstExpression() override;

  protected Q_SLOTS:
    void serverChangedStatus(int status);
    void receiveSymbols(const QStringList& vars, const QStringList& values, const QStringList & funcs);
    void fillSyntaxRegExps(QVector<QRegExp>& v, QVector<QRegExp>& f);

  Q_SIGNALS:
    void symbolsChanged();

  private:
    QProcess* m_process;
    org::kde::Cantor::R* m_rServer;

    /* Available variables and functions, TODO make full classes and type info */
    Cantor::DefaultVariableModel* m_variableModel;
    QStringList m_variables;
    QStringList m_functions;
};

#endif /* _RSESSION_H */
