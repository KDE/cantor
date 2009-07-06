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

#ifndef _TABCOMPLETIONOBJECT_H
#define _TABCOMPLETIONOBJECT_H

#include <kcompletion.h>

#include "mathematik_export.h"

namespace MathematiK
{
class TabCompletionObjectPrivate;
class Session;

class MATHEMATIK_EXPORT TabCompletionObject : public KCompletion
{
  Q_OBJECT
  public:
    TabCompletionObject( const QString& command, Session* parent);
    ~TabCompletionObject();

    QStringList completions();
    QString command();
    Session* session();
  protected:
    void setCompletions(const QStringList& completions);
  protected Q_SLOTS:
    virtual void fetchCompletions() = 0;
  Q_SIGNALS:
    void done();
  private:
    TabCompletionObjectPrivate* d;
};

}

#endif /* _TABCOMPLETIONOBJECT_H */
