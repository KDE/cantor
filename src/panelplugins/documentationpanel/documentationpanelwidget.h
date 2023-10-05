/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2020 Shubham <aryan100jangid@gmail.com>
    SPDX-FileCopyrightText: 2020-2021 Alexander Semke <alexander.semke@web.de>
*/

#ifndef _DOCUMENTATIONPANELWIDGET_H
#define _DOCUMENTATIONPANELWIDGET_H

#include <QBuffer>
#include <QHelpEngine>
#include <QWebEngineUrlRequestJob>
#include <QWebEngineUrlSchemeHandler>
#include <QWidget>

class QComboBox;
class QHelpContentWidget;
class QHelpIndexWidget;
class QLineEdit;
class QStackedWidget;
class QToolButton;
class QUrl;
class QWebEngineDownloadRequest;
class QWebEngineView;

class DocumentationPanelWidget : public QWidget
{
  Q_OBJECT

  public:
    DocumentationPanelWidget(QWidget*);
    ~DocumentationPanelWidget();

    void updateBackend(const QString&);
    QUrl url() const;

  public Q_SLOTS:
    void showUrl(const QUrl&);

  Q_SIGNALS:
    void zoomFactorChanged();

  private Q_SLOTS:
    void contextSensitiveHelp(const QString&);
    void returnPressed();

    // SLOTS for Find in Page widget
    void searchForward();
    void searchBackward();

    void downloadResource(QWebEngineDownloadRequest *); // slot for saving the image or html to local disk

  private:
    void updateDocumentation();

    QHelpEngine* m_engine = nullptr;
    QWebEngineView* m_webEngineView = nullptr;
    QStackedWidget* m_stackedWidget = nullptr;
    QHelpIndexWidget* m_indexWidget = nullptr;
    QHelpContentWidget* m_contentWidget = nullptr;
    QString m_backend;
    QStringList m_docNames;
    QStringList m_docPaths;
    bool m_initializing = false;

    // member variables for find in page text widget
    QLineEdit* m_search = nullptr; // for searching through keywords
    QLineEdit* m_findText = nullptr; // for find in page text widget
    QToolButton* m_matchCase = nullptr;

    QComboBox* m_documentationSelector = nullptr;
    QString m_currentQchFileName;
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
