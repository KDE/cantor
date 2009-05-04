/*
    Tims program is free software; you can redistribute it and/or
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

#ifndef _MAXIMASESSION_H
#define _MAXIMASESSION_H

#include "session.h"
#include "expression.h"

#include <kdirwatch.h>
#include <QRegExp>

class MaximaExpression;
class KPtyProcess;


class MaximaSession : public MathematiK::Session
{
  Q_OBJECT
  public:
    static const QRegExp MaximaPrompt;
    static const QRegExp MaximaOutputPrompt;

    MaximaSession( MathematiK::Backend* backend);
    ~MaximaSession();

    void login();
    void logout();

    MathematiK::Expression* evaluateExpression(const QString& command);

    void appendExpressionToQueue(MaximaExpression* expr);
    void runCommand(const QByteArray& command);

    void interrupt();

    void sendSignalToProcess(int signal);

    MathematiK::Expression* contextHelp(const QString& command);

  public slots:
    void readStdOut();
    void readStdErr();

  private slots:
    void currentExpressionChangedStatus(MathematiK::Expression::Status status);

  private:
    void runFirstExpression();
  private:
    KPtyProcess* m_process;
    QList<MaximaExpression*> m_expressionQueue;
    bool m_isInitialized;
    QString m_tmpPath;
    KDirWatch m_dirWatch;
};

#endif /* _MAXIMASESSION_H */
