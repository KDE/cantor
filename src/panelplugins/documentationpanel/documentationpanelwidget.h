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
    Copyright (C) 2020 Shubham <aryan100jangid@gmail.com>
 */

#ifndef _DOCUMENTATIONPANELWIDGET_H
#define _DOCUMENTATIONPANELWIDGET_H

#include <QPointer>
#include <QWidget>

namespace Cantor{
class Session;
}

class QHelpEngine;

class DocumentationPanelWidget : public QWidget
{
  Q_OBJECT
  public:
    DocumentationPanelWidget( Cantor::Session* session, QWidget* parent );
    ~DocumentationPanelWidget() override = default;

    void setSession(Cantor::Session* session);
    void addWidgets();

    /** @return icon of the current backend **/
    QIcon icon() const;

    /** @return name of the current backend **/
    QString name() const;

    void loadDocumentation();
    /** @return local paths to all QCH files found in cantor/admin/documentation directory **/

    QStringList qchFiles() const;

  private:
    Cantor::Session* m_session;
    QPointer<QHelpEngine> m_engine;
    QString m_path; // path to local QCH files
};

#endif /* _DOCUMENTATIONPANELWIDGET_H */
