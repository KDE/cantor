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


#ifndef MATHEMATIK_H
#define MATHEMATIK_H

#include <kparts/mainwindow.h>

#include <QList>

class KTabWidget;
class KTextEdit;

/**
 * This is the application "Shell".  It has a menubar, toolbar, and
 * statusbar but relies on the "Part" to do all the real work.
 *
 * @short Application Shell
 * @author ${AUTHOR} <${EMAIL}>
 * @version ${APP_VERSION}
 */
class MathematiKShell : public KParts::MainWindow
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    MathematiKShell();

    /**
     * Default Destructor
     */
    virtual ~MathematiKShell();

    /**
     * Use this method to load whatever file/URL you have
     */
    void load(const KUrl& url);

protected:
    /**
     * This method is called when it is time for the app to save its
     * properties for session management purposes.
     */
    void saveProperties(KConfigGroup &);

    /**
     * This method is called when this app is restored.  The KConfig
     * object points to the session management config file that was saved
     * with @ref saveProperties
     */
    void readProperties(const KConfigGroup &);

private slots:
    void fileNew();
    void fileOpen();
    void optionsConfigureKeys();
    void optionsConfigureToolbars();

    void applyNewToolbarConfig();

    void addWorksheet();
    void addWorksheet(const QString& backendName);
    
    void activateWorksheet(int index);

    void setTabCaption(const QString& tab);
    void closeTab(QWidget* widget = 0);

    void showSettings();

    void downloadExamples();
    void openExample();

private:
    void setupActions();

private:
    QList<KParts::ReadWritePart *> m_parts;
    KParts::ReadWritePart* m_part;
    KTabWidget* m_tabWidget;
    KTextEdit* m_helpView;
};

#endif // MATHEMATIK_H
