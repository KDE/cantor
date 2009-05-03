#ifndef MATHEMATIK_H
#define MATHEMATIK_H

#include <kparts/mainwindow.h>

#include <QList>

class KTabWidget;
class QTextEdit;

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

    void addWorksheet(const QString& backendName="nullbackend");
    
    void activateWorksheet(int index);

    void setTabCaption(const QString& tab);

    void showSettings();

private:
    void setupActions();

private:
    QList<KParts::ReadWritePart *> m_parts;
    KParts::ReadWritePart* m_part;
    KTabWidget* m_tabWidget;
    QTextEdit* m_helpView;
};

#endif // MATHEMATIK_H
