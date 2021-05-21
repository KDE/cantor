/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _BACKENDCHOOSEDIALOG_H
#define _BACKENDCHOOSEDIALOG_H

#include <QDialog>

#include <ui_backendchooser.h>

class BackendChooseDialog : public QDialog
{
  Q_OBJECT
  public:
    explicit BackendChooseDialog(QWidget* parent);
    ~BackendChooseDialog() override;

    QString backendName();

  protected Q_SLOTS:
    void onAccept();
    void updateContent();

  private:
    Ui::BackendChooserBase m_ui;
    QString m_backend;
};

#endif /* _BACKENDCHOOSEDIALOG_H */
