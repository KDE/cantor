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

#include <QBuffer>
#include <QHelpEngine>
#include <QWebEngineUrlRequestJob>
#include <QWebEngineUrlSchemeHandler>
#include <QWidget>

namespace Cantor
{
    class Session;
}

class QHelpEngine;
class QHelpIndexWidget;
class QLineEdit;
class QStackedWidget;
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

  Q_SIGNALS:
    void activateBrowser();

  private Q_SLOTS:
    void displayHelp(const QUrl&);
    void contextSensitiveHelp(const QString&);
    void returnPressed();

  private:
    QHelpEngine* m_engine = nullptr;
    QWebEngineView* m_textBrowser = nullptr;
    QStackedWidget* m_displayArea = nullptr;
    QHelpIndexWidget* m_index = nullptr;
    QLineEdit* m_search = nullptr;
    QString m_backend;
};

// class for handling of custom url scheme ie. qthelp:// inside QWebEngineView
class QtHelpSchemeHandler : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT

  public:
    QtHelpSchemeHandler(QHelpEngine* helpEngine) : m_HelpEngine(helpEngine)
    {
    }

    virtual void requestStarted(QWebEngineUrlRequestJob* job) override
    {
        auto url = job->requestUrl();
        auto data = new QByteArray;
        *data = m_HelpEngine->fileData(url);
        auto buffer = new QBuffer(data);
        if (url.scheme() == QLatin1String("qthelp")) {
            job->reply("text/html", buffer);
        }
    }

  private:
    QHelpEngine* m_HelpEngine;
};

#endif /* _DOCUMENTATIONPANELWIDGET_H */
