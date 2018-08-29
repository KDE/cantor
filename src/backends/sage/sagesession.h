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

#ifndef _SAGESESSION_H
#define _SAGESESSION_H

#include "session.h"
#include "expression.h"

#include <KDirWatch>
#include <QProcess>

class SageExpression;
class KPtyProcess;


class SageSession : public Cantor::Session
{
  Q_OBJECT
  public:
    static const QByteArray SagePrompt;
    static const QByteArray SageAlternativePrompt;

    //small helper class to deal with sage versions
    //Note: major version -1 is treated as most current
    class VersionInfo{
      public:
        VersionInfo(int major = -1, int minor = -1);

        //bool operator <=(VersionInfo v2);
        bool operator <(VersionInfo other) const;
        bool operator <=(VersionInfo other) const;
        bool operator >(VersionInfo other) const;
        bool operator >=(VersionInfo other) const;
        bool operator ==( VersionInfo other) const;

        // These are not called major() and minor() because some libc's have
        // macros with those names.
        int majorVersion() const;
        int minorVersion() const;
      private:
        int m_major;
        int m_minor;
    };


    SageSession( Cantor::Backend* backend);
    ~SageSession() override = default;

    void login() override;
    void logout() override;

    Cantor::Expression* evaluateExpression(const QString& command,Cantor::Expression::FinishingBehavior behave = Cantor::Expression::FinishingBehavior::DoNotDelete, bool internal = false) override;

    void runFirstExpression() override;

    void interrupt() override;

    void sendSignalToProcess(int signal);
    void sendInputToProcess(const QString& input);
    void waitForNextPrompt();

    void setTypesettingEnabled(bool enable) override;
    void setWorksheetPath(const QString& path) override;

    Cantor::CompletionObject* completionFor(const QString& command, int index=-1) override;
    QSyntaxHighlighter* syntaxHighlighter(QObject* parent) override;

    VersionInfo sageVersion();
  public Q_SLOTS:
    void readStdOut();
    void readStdErr();

  private Q_SLOTS:
    void currentExpressionChangedStatus(Cantor::Expression::Status status);
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void reportProcessError(QProcess::ProcessError error);
    void fileCreated(const QString& path);

  private:
    void defineCustomFunctions();
    bool updateSageVersion();
  private:
    KPtyProcess* m_process;
    int m_isInitialized;
    QString m_tmpPath;
    KDirWatch m_dirWatch;
    bool m_waitingForPrompt;
    QString m_outputCache;
    VersionInfo m_sageVersion;
    bool m_haveSentInitCmd;
};

#endif /* _SAGESESSION_H */
