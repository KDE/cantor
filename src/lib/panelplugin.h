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
    Copyright (C) 2010 Alexander Rieder <alexanderrieder@gmail.com>
 */

#ifndef _PANEL_PLUGIN_H
#define _PANEL_PLUGIN_H

#include <QObject>
class KPluginMetaData;

#include "backend.h"

#include "cantor_export.h"

namespace Cantor
{
class Session;
class PanelPluginPrivate;

/**
 * A plugin provides some additional features for the worksheet
 */
class CANTOR_EXPORT PanelPlugin : public QObject
{
  Q_OBJECT
  public:
    struct State {
        Session* session{nullptr};
        QVector<QVariant> inners;
    };


    /**
     * Create a new PanelPlugin
     * @param parent the parent Object @see QObject
     **/
    PanelPlugin( QObject* parent );
    /**
     * Destructor
     */
    ~PanelPlugin() override;

    /**
     * Sets the properties of this PanelPlugin
     * according to KPluginMetaData
     * @param info KPluginMetaData
     */
    void setPluginInfo(const KPluginMetaData&);

    /**
     * Returns a list of all extensions, the current backend
     * must provide to make this PanelPlugin work. If it doesn't
     * this PanelPlugin won't be enabled
     * @return list of required extensions
    */
    QStringList requiredExtensions();


    /**
     * Returns the capabilities, the current backend
     * must provide to make this PanelPlugin work. If it doesn't
     * this PanelPlugin won't be enabled
     * @return the required capabilities
    */
    virtual Backend::Capabilities requiredCapabilities();


    /**
     * Returns the name of the plugin
     * @return name of the plugin
     */
    QString name();

    /**
     * returns the widget, provided by this plugin
     * @return the widget, provided by this plugin
     **/
    virtual QWidget* widget() = 0;

    void setParentWidget(QWidget* widget);
    QWidget* parentWidget();

    /**
     * Save state of panel to storable form
     *
     **/
    virtual State saveState();

    /**
     * Restore state
     * Can contains only session - this is init state from Cantor shell
     */
    virtual void restoreState(const State& state);

    /**
     * For proper connection to Cantor shell. All connections should be done here
     */
    virtual void connectToShell(QObject* cantorShell);

    /**
     * Show on worksheet startup or not
     * Default returns true
     */
    virtual bool showOnStartup();

  protected:
    Session* session();

  Q_SIGNALS:
    void requestRunCommand(const QString& cmd);
    void visibilityRequested();

  private:
    PanelPluginPrivate* d;
};

}

#endif /* _PANEL_PLUGIN_H */
