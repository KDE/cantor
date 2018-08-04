/*
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>

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
 */


#ifndef CANTOR_H
#define CANTOR_H

#include <QObject>
#include <KParts/MainWindow>

#include <QList>

class QTabWidget;
class KTextEdit;

namespace Cantor{
class WorksheetAccessInterface;
}

namespace KParts{
    class ReadWritePart;
}

/**
 * This is the application "Shell".  It has a menubar, toolbar, and
 * statusbar but relies on the "Part" to do all the real work.
 */
class CantorShell : public KParts::MainWindow
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    CantorShell();

    /**
     * Default Destructor
     */
    ~CantorShell() override;

    /**
     * Use this method to load whatever file/URL you have
     */
    void load(const QUrl& url);

    /**
     * checks if at least one usable Backend is installed
     */
    bool hasAvailableBackend();

    Cantor::WorksheetAccessInterface* currentWorksheetAccessInterface();
protected:
    /**
     * This method is called when it is time for the app to save its
     * properties for session management purposes.
     */
    void saveProperties(KConfigGroup &) Q_DECL_OVERRIDE;

    /**
     * This method is called when this app is restored.  The KConfig
     * object points to the session management config file that was saved
     * with @ref saveProperties
     */
    void readProperties(const KConfigGroup &) Q_DECL_OVERRIDE;
public Q_SLOTS:
    void addWorksheet();
    void addWorksheet(const QString& backendName);

private Q_SLOTS:
    void fileNew();
    void fileOpen();
    void optionsConfigureKeys();

    void activateWorksheet(int index);

    void setTabCaption(const QString& tab, const QIcon& icon);
    void closeTab(int index = -1);

    void showSettings();

    void downloadExamples();
    void openExample();

    void updatePanel();
private:
    void setupActions();
    void closeEvent(QCloseEvent*) Q_DECL_OVERRIDE;
    bool reallyClose(bool checkAllParts = true);
    KParts::ReadWritePart* findPart(QWidget* widget);

private:
    QMap<KParts::ReadWritePart*, QStringList> m_pluginsVisibility;
    QList<KParts::ReadWritePart *> m_parts;
    KParts::ReadWritePart* m_part;
    QTabWidget* m_tabWidget;
    QList<QDockWidget*> m_panels;
    QDockWidget* m_helpDocker;
};

#endif // CANTOR_H
