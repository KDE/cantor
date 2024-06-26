/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
*/

#ifndef _SAGESESSION_H
#define _SAGESESSION_H

#include "session.h"
#include "expression.h"

#include <KDirWatch>
#include <QProcess>

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
        explicit VersionInfo(int major = -1, int minor = -1);

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

    explicit SageSession(Cantor::Backend*);
    ~SageSession() override;

    void login() override;
    void logout() override;

    Cantor::Expression* evaluateExpression(const QString& command,Cantor::Expression::FinishingBehavior behave = Cantor::Expression::FinishingBehavior::DoNotDelete, bool internal = false) override;

    void runFirstExpression() override;

    void interrupt() override;

    void sendInputToProcess(const QString& input);

    void setTypesettingEnabled(bool) override;

    Cantor::CompletionObject* completionFor(const QString& command, int index=-1) override;
    QSyntaxHighlighter* syntaxHighlighter(QObject* parent) override;

    VersionInfo sageVersion();

  public Q_SLOTS:
    void readStdOut();
    void readStdErr();

  private Q_SLOTS:
    void processFinished(int exitCode, QProcess::ExitStatus);
    void reportProcessError(QProcess::ProcessError);
    void fileCreated(const QString& path);

  private:
    void defineCustomFunctions();
    bool updateSageVersion();

    QProcess* m_process{nullptr};
    bool m_isInitialized{false};
    QString m_tmpPath;
    KDirWatch m_dirWatch;
    bool m_waitingForPrompt{false};
    QString m_outputCache;
    VersionInfo m_sageVersion;
    bool m_haveSentInitCmd{false};
};

#endif /* _SAGESESSION_H */
