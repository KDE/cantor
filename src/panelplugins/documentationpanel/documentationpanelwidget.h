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

#include <QWidget>

namespace Cantor{
class Session;
}

class QHelpEngine;
class QUrl;
class QWebEngineView;

class DocumentationPanelWidget : public QWidget
{
  Q_OBJECT

  public:
    DocumentationPanelWidget(Cantor::Session* session, QWidget* parent);
    ~DocumentationPanelWidget();

    void setSession(Cantor::Session* session);

    /** @return name of the current backend **/
    QString backendName() const;

    void loadDocumentation();

  public:
    Cantor::Session* m_session = nullptr;

  private Q_SLOTS:
    void displayHelp(const QUrl&);
    void doSearch(const QString&);
    void contextSensitiveHelp(const QString&);

  private:
    QHelpEngine* m_engine = nullptr;
    QWebEngineView* m_textBrowser = nullptr;
    QString m_backend;
};

#endif /* _DOCUMENTATIONPANELWIDGET_H */
