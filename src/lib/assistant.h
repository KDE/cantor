/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _ASSISTANT_H
#define _ASSISTANT_H

#include <KXMLGUIClient>
#include <QObject>
#include <KPluginMetaData>

#include "cantor_export.h"

namespace Cantor
{
class Backend;
class AssistantPrivate;

/**
 * An Assistant is a dialog for simplifying common tasks, like integrating, solving, or running scripts
 * To perform their task, they rely on one or more Extensions, to translate to the backends specific syntax.
 * @see Extension
 */
class CANTOR_EXPORT Assistant : public QObject, public KXMLGUIClient
{
  Q_OBJECT
  public:
    /**
     * Create a new assistant
     * @param parent the parent Object @see QObject
     **/
    explicit Assistant( QObject* parent );
    /**
     * Destructor
     */
    ~Assistant() override;

    /**
     * Sets the backend, this Assistant operates on
     * @param backend the new backend
     */
    void setBackend(Backend* backend);

    /**
     * Sets the properties of this Assistant
     * according to KPluginMetaData
     * @param info KPluginMetaData
     */
    void setPluginInfo(const KPluginMetaData &info);

    /**
     * Returns a list of all extensions, the current backend
     * must provide to make this Assistant work. If it doesn't
     * this Assistant won't be shown in the Menu
     * @return list of required extensions
    */
    QStringList requiredExtensions();

    /**
     * shows the assistants dialog or gui it offers, and returns a list of commands
     * to be run, to achieve the desired effect
     * @param parent the parent widget, each created Widget should use
     */
    virtual QStringList run(QWidget* parent) = 0;

    /**
     * initialize the needed KActions/integrate into the menu bars
     */
    virtual void initActions() = 0;

    /**
     * Returns the icon, this Assistant is using
     * @return icon, this Assistant is using
     */
    QString icon();
    /**
     * Returns the name of the assistant
     * @return name of the assistant
     */
    QString name();
    /**
     * Returns the backend, this assistant operates on
     * @return backend, this assistant operates on
     */
    Backend* backend();

  Q_SIGNALS:
    /**
     * signal emitted, if the user has requested this Assistant to run
     * e.g. by clicking on its action in the menu
     */
    void requested();

  private:
    AssistantPrivate* d;
};

}
#endif /* _ASSISTANT_H */
