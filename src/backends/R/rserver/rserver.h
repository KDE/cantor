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

#ifndef _RSERVER_H
#define _RSERVER_H

#include <QObject>

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
  Q_CLASSINFO("D-Bus Interface", "org.kde.MathematiK.R")

  public:
    enum Status { Idle=0, Busy };
    enum ReturnCode { SuccessCode=0, ErrorCode, InterruptedCode};
    RServer( );
    ~RServer();

    void initR();
    void autoload();
    void endR();

    QString requestInput(const QString& prompt);
    void showFiles(const QStringList& files);

  signals:
    void ready();
    void statusChanged(int status);
    void expressionFinished(int returnCode, const QString& text);
    void showFilesNeeded(const QStringList& files);
    void inputRequested(const QString& prompt);

    void requestAnswered();
    
  public slots:
    void runCommand(const QString& cmd, bool internal=false);
    void answerRequest(const QString& answer);
  private:
    void setStatus(Status status);
    void newPlotDevice();
  private:
    bool m_isInitialized;
    Status m_status;
    QString m_requestCache;
    QString m_tmpDir;
    QString m_curPlotFile;   
};

#endif /* _RSERVER_H */
