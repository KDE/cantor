/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2015 Alexander Rieder <alexanderrieder@gmail.com>
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
    explicit WorksheetAccessInterface(QObject* parent);
    ~WorksheetAccessInterface() override = default;
  public:
    virtual QByteArray saveWorksheetToByteArray() = 0;
    virtual void loadWorksheetFromByteArray(QByteArray* data) = 0;

    virtual Session* session() = 0;
  public Q_SLOTS:
      virtual void evaluate() = 0;
      virtual void interrupt() = 0;

  Q_SIGNALS:
      void modified();
};

}

#endif




