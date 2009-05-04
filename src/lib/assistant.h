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

#ifndef _ASSISTANT_H
#define _ASSISTANT_H

#include <kxmlguiclient.h>
#include <QObject>
#include <kplugininfo.h>

#include "mathematik_export.h"

namespace MathematiK
{
class Backend;
class AssistantPrivate;

class MATHEMATIK_EXPORT Assistant : public QObject, public KXMLGUIClient
{
  Q_OBJECT
  public:
    Assistant( QObject* parent );
    ~Assistant();

    void setBackend(Backend* b);

    void setPluginInfo(KPluginInfo info);
    
    /**Returns a list of all extensions, the current backend
      must provide to make this Assistant work. If it doesn't
      this Assistant won't be shown in the Menu
    **/
    QStringList requiredExtensions();

    /**shows the assistants dialog, and returns a list of commands
       to be run, to achieve the desired effect
    **/
    virtual QStringList run(QWidget* parent) = 0;

    virtual void initActions() = 0;

    QString icon();
    QString name();
    Backend* backend();

  Q_SIGNALS:
    void requested();

  private:
    AssistantPrivate* d;
};

}
#endif /* _ASSISTANT_H */
