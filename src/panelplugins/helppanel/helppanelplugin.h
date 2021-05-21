/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2010 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _HELPPANELPLUGIN_H
#define _HELPPANELPLUGIN_H

#include "panelplugin.h"

class KTextEdit;

class HelpPanelPlugin : public Cantor::PanelPlugin
{
  Q_OBJECT
  public:
    HelpPanelPlugin(QObject* parent, const QList<QVariant>& args);
    ~HelpPanelPlugin() override;

    QWidget* widget() override;

    bool showOnStartup() override;

    void connectToShell(QObject * cantorShell) override;

    Cantor::PanelPlugin::State saveState() override;

    void restoreState(const Cantor::PanelPlugin::State & state) override;

  public Q_SLOTS:
    void setHelpHtml(const QString&);
    void showHelp(const QString&);

  private:
    QPointer<KTextEdit> m_edit;

};

#endif /* _HELPPANELPLUGIN_H */
