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
    Copyright (C) 2015 Alexander Rieder <alexanderrieder@gmail.com>
 */

#ifndef _WORKSHEET_ACCESS_INTERFACE_H
#define _WORKSHEET_ACCESS_INTERFACE_H

#include "cantor_export.h"

#include<QObject>


namespace Cantor
{
    class Session;
    
class CANTOR_EXPORT WorksheetAccessInterface : public QObject
{
  Q_OBJECT
  public:  
    static QLatin1String Name;
    WorksheetAccessInterface(QObject* parent);
    ~WorksheetAccessInterface() override;
  public:
    virtual QByteArray saveWorksheetToByteArray() = 0;
    virtual void loadWorksheetFromByteArray(QByteArray* data) = 0;

    virtual Session* session() = 0;
  public Q_SLOTS:
      virtual void evaluate() = 0;
      virtual void interrupt() = 0;

  Q_SIGNALS:
      void sessionChanged();
};

}

#endif




