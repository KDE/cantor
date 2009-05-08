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

#ifndef _BACKEND_H
#define _BACKEND_H

#include <QObject>
#include <QVariant>
#include <kxmlguiclient.h>
#include <kplugininfo.h>

#include "mathematik_export.h"

class KConfigSkeleton;
class QWidget;

namespace MathematiK
{
class Session;
class Extension;
class BackendPrivate;

class MATHEMATIK_EXPORT Backend : public QObject, public KXMLGUIClient
{
  Q_OBJECT
  public:
    enum Capability{
        Nothing = 0x0,
        LaTexOutput = 0x1
    };
    Q_DECLARE_FLAGS(Capabilities, Capability)


    Backend( QObject* parent = 0,const QList<QVariant> args=QList<QVariant>() );
    virtual ~Backend();

    virtual Session* createSession() = 0;
    virtual Capabilities capabilities() = 0; 

    //Stuff extracted from the .desktop file
    QString name();
    QString description();
    QString icon();
    
    virtual QWidget* settingsWidget(QWidget* parent);
    virtual KConfigSkeleton* config();

    QStringList extensions();
    Extension * extension(const QString& name);

    static QStringList listAvailableBackends();
    static QList<KPluginInfo> availableBackendInformations();
    static Backend* createBackend(const QString& name,QObject* parent);
  private:
    BackendPrivate* d;
};

}
#endif /* _BACKEND_H */
