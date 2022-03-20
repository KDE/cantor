/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Filipe Saraiva <filipe@kde.org>
    SPDX-FileCopyrightText: 2015 Minh Ngo <minh@fedoraproject.org>
*/

#ifndef _PYTHONSESSION_H
#define _PYTHONSESSION_H

#include "session.h"
#include <QStringList>
#include <QProcess>

class PythonSession : public Cantor::Session
{
  Q_OBJECT
  public:
    PythonSession(Cantor::Backend*);
    ~PythonSession() override;

    void login() override;
    void logout() override;

    void interrupt() override;

    Cantor::Expression* evaluateExpression(const QString& command, Cantor::Expression::FinishingBehavior behave = Cantor::Expression::FinishingBehavior::DoNotDelete, bool internal = false) override;
    Cantor::CompletionObject* completionFor(const QString& command, int index=-1) override;
    QSyntaxHighlighter* syntaxHighlighter(QObject* parent) override;
    void setWorksheetPath(const QString&) override;

    QString plotFilePrefixPath();
    int& plotFileCounter();

  private:
    QProcess* m_process{nullptr};
    QString m_worksheetPath;
    QString m_output;
    QString m_plotFilePrefixPath;
    int m_plotFileCounter{0};

  private Q_SLOT:
    void readOutput();
    void reportServerProcessError(QProcess::ProcessError);

  private:
    void runFirstExpression() override;
    void updateGraphicPackagesFromSettings();
    QString graphicPackageErrorMessage(QString packageId) const override;

    void sendCommand(const QString& command, const QStringList arguments = QStringList()) const;
};

#endif /* _PYTHONSESSION_H */
