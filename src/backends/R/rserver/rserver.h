/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _RSERVER_H
#define _RSERVER_H

#include <QObject>
#include <QChar>
#include <QMap>
#include <QString>
#include <QStringList>

class Expression
{
  public:
    QString cmd;
    int returnCode;
    bool hasOtherResults;
    QString err_buffer;
    QString std_buffer;
};

class RServer : public QObject
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "org.kde.Cantor.R")

  public:
    enum Status { Idle=0, Busy };
    enum ReturnCode { SuccessCode=0, ErrorCode, InterruptedCode};
    RServer( );
    ~RServer() override;

    void initR();
    void autoload();
    void endR();

    QString requestInput(const QString& prompt);
    void addFileToOutput(const QString& file);

  Q_SIGNALS:
    void ready();
    void statusChanged(int status);
    void expressionFinished(int returnCode, const QString& text, const QStringList& files);
    void inputRequested(const QString& prompt);

    void requestAnswered();

  public Q_SLOTS:
    void runCommand(const QString& cmd, bool internal=false);
    void answerRequest(const QString& answer);

  private:
    struct CachedParsedNamespace {
        QStringList functions;
        QStringList constants;
    };

  private:
    void setStatus(Status status);
    void newPlotDevice();
    void completeCommand(const QString& cmd); // TODO: comment properly, only takes command from start to cursor
    void listSymbols();

  private:
    const static QChar recordSep;
    const static QChar unitSep;

  private:
    bool m_isInitialized;
    bool m_isCompletionAvailable;
    Status m_status;
    QString m_requestCache;
    QString m_tmpDir;
    QString m_curPlotFile;
    QStringList m_expressionFiles;
    QMap<QString, CachedParsedNamespace> m_parsedNamespaces;
};

#endif /* _RSERVER_H */
