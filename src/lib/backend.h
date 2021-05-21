/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _BACKEND_H
#define _BACKEND_H

#include <QObject>
#include <QVariant>
#include <KXMLGUIClient>
#include <KPluginInfo>

#include "graphicpackage.h"

#include "cantor_export.h"

class KConfigSkeleton;
class QWidget;

/**
 * Namespace collecting all Classes of the Cantor Libraries
 */
namespace Cantor
{
class Session;
class Extension;
class BackendPrivate;

/**
 * The Backend class provides access to information about the backend.
 * It provides access to what features are supported by the backend, and
 * a factory method to create a new Session
 * It needs to be subclassed by all Backends.
 *
 * @author Alexander Rieder
 */

class CANTOR_EXPORT Backend : public QObject, public KXMLGUIClient
{
  Q_OBJECT
  public:
    /**
     * This enum is used to specify the Features, supported by a backend.
     */
    enum Capability{
        Nothing = 0x0,             ///< the Backend doesn't support any of the optional features
        LaTexOutput = 0x1,         ///< it can output results as LaTeX code
        InteractiveMode = 0x2,     /**< it supports an interactive workflow. (It means a command
                                        can ask for additional input while running)
                                   */
        SyntaxHighlighting = 0x4,  ///< it offers a custom Syntax Highlighter
        Completion = 0x8,          ///< it offers completion of partially typed commands
        SyntaxHelp = 0x10,         /**< it offers help about a commands syntax, that will
                                        be shown in a tooltip
                                   */
        VariableManagement = 0x20, ///< it offers access to the variables (for variable management panel)
        IntegratedPlots = 0x40,    ///< it offers, that backend supports plot not only as separate window, but also image result
    };
    Q_DECLARE_FLAGS(Capabilities, Capability)

  protected:
    /**
     * Create a new Backend. Normally the static createBackend factory method
     * should be used.
     * @param parent the Parent object
     * @param args optional arguments (not used)
     */
    explicit Backend( QObject* parent = nullptr,const QList<QVariant>& args=QList<QVariant>() );
    /**
     * Destructor. Doesn't anything.
     */
    ~Backend() override;

  public:

    /**
     * Creates a new Session. It is the way to go to create a Session,
     * don't call new Session on your own.
     * @return a new Session of this Backend, or 0 if creating failed
     */
    virtual Session* createSession() = 0;
    /**
     * Returns list of the supported optional features
     * @return a list of features, containing items of the Capability enum, ORed together
     */
    virtual Capabilities capabilities() const = 0;
    /**
     * Returns whether all of this backends requirements are fulfilled, or if some are missing.
     * @param reason if set, backend write information about missing requirements, if they exists
     * @return @c true if all the requirements needed to use this Backend are fulfilled
     * @return @c false some requirements are missing. e.g. the maxima executable can not be found
     * @see Capability
    */
    virtual bool requirementsFullfilled(QString* const reason = nullptr) const = 0;

    /**
     * Returns a unique string to identify this backend.
     * In contrast to name() this string isn't translated
     * @return string to identify backend
     */
    virtual QString id() const = 0;

    /**
     * Returns the recommended version of the backend supported by Cantor
     * @return the recommended version of the backend
     */
    virtual QString version() const = 0;

    //Stuff extracted from the .desktop file
    /**
     * Returns the name of the backend
     * @return the backends name
     */
    QString name() const;

    /**
     * Returns a short comment about the backend.
     * @return comment about the backend
     */
    QString comment() const;
    /**
     * Returns the icon to use with this backend
     * @return name of the icon
     */
    QString icon() const;
    /**
     * Returns the Url of the Homepage for the Backend
     * @return the url
     */
    QString url() const;
    /**
     * Returns an Url pointing to the Help of the Backend
     * The method should be overwritten by all Backends(who have an online help)
     * You should make the returned Url translatable, e.g. by doing something like:
     * return i18nc("the url to the documentation of KAlgebra, please check if there is a translated version and use the correct url",
     *   "https://docs.kde.org/?application=kalgebra");
     * @return Url of the help
     */
    virtual QUrl helpUrl() const = 0;
    /**
     * Returns if the backend should be enabled (shown in the Backend dialog)
     * @return @c true, if the enabled flag is set to true, and the requirements are fulfilled
     * @return @c false, if the backend was purposely disabled, or requirements are missing
     * @see requirementsFullfilled()
     */
    bool isEnabled() const;
    /**
     * Enables/disables this backend
     * @param enabled true to enable backend false to disable
     */
    void setEnabled(bool enabled);

    /**
     * Returns a longer description of the Backend, e.g. purpose, strengths etc.
     * It should help the user to decide between the different Backends
     * @return a description of the backend. It can contain html
     */
    virtual QString description() const;
    /**
     * Returns a Widget for configuring this backend
     * @return Widget for usage in the Settings dialog
     */
    virtual QWidget* settingsWidget(QWidget* parent) const = 0;
    /**
     * Returns a KConfig object, containing all the settings,
     * the backend might need
     * @return a KConfigSkeleton object, for configuring this backend
     */
    virtual KConfigSkeleton* config() const = 0;
    /**
     * Returns a list of the names of all the Extensions supported by this backend
     * @return a list of the names of all the Extensions supported by this backend
     * @see extension(const QString& name)
     */
    QStringList extensions() const;
    /**
     * Returns an Extension of this backend for the given name, or null
     * if the Backend doesn't have an extension with this name.
     * @return Pointer to the Extension object with the given name
     */
    Extension* extension(const QString& name) const;

    /**
     * Returns a list of the names of all the installed and enabled backends
     * @return a list of the names of all the installed and enabled backends
     * @see isEnabled()
     */
    static QStringList listAvailableBackends();
    /**
     * Returns Pointers to all the installed backends
     * @return Pointers to all the installed backends
     */
    static QList<Backend*> availableBackends();
    /**
     * Returns the backend with the given name, or null if it isn't found
     * @return the backend with the given name, or null if it isn't found
     */
    static Backend* getBackend(const QString& name);

    /**
     * @return @c true if all the requirements (the path is correct, the file is executable, etc.) are fulfilled
     * for the backend @c Name with the path to the executable @c path and false otherwise.
     * In case the requrements are not fulfilled, the reason is written to @c reason.
     */
    static bool checkExecutable(const QString& name, const QString& path, QString* reason = nullptr);

    /**
     * This is test function, which allow test, that run @p program with @p args produce output file @p filename
     * with content @p expectedContent during time less that @p timeOut.
     * If something go wrong, problem will be described in @p reason, if the parameter is not @p nullptr.
     */
    static bool testProgramWritable(
        const QString& program, const QStringList& args, const QString& filename, const QString& expectedContent, QString* reason = nullptr, int timeOut = 5000
    );

    /**
     * This function will return list of all available workable graphics packages for integrated graphics
     */
    QList<GraphicPackage> availableGraphicPackages() const;

  private:
    BackendPrivate* d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Backend::Capabilities)

}

#endif /* _BACKEND_H */
