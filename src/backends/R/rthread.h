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

#ifndef _RTHREAD_H
#define _RTHREAD_H

#include <QThread>

class RSession;
class RExpression;

class RThread : public QThread
{
  Q_OBJECT
  public:
    RThread( RSession* parent);
    ~RThread();
    
    void run();

    bool isBusy();

  public slots:
    void queueExpression(RExpression* expression);
  signals:
    void ready();
    void expressionFinished(RExpression* expression);
  protected slots:
    void evaluateExpression();

  protected:
    void initR();
    void autoload();
    void endR();

  private:
    RSession* m_session;
    QList<RExpression* > m_queue;
};

#endif /* _RTHREAD_H */
