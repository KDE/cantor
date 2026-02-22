/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _SCRIPTEDITORWIDGET_H
#define _SCRIPTEDITORWIDGET_H

#include <KXmlGuiWindow>

class QTemporaryFile;

namespace KTextEditor
{
    class View;
    class Document;
}

class ScriptEditorWidget : public KXmlGuiWindow
{
  Q_OBJECT
  public:
    explicit ScriptEditorWidget( const QString& filter, const QString& highlightingMode, QWidget* parent = nullptr );
    ~ScriptEditorWidget() override;
    void open(const QUrl &url);

  Q_SIGNALS:
    void runScript(const QString& filename);

  private Q_SLOTS:
    void newScript();
    void open();
    void run();
    void updateCaption();

  protected:
    bool queryClose() override;

  private:
    QString m_filter;
    KTextEditor::View* m_editor = nullptr;
    KTextEditor::Document* m_script = nullptr;
    QTemporaryFile* m_tmpFile = nullptr;
};

#endif /* _SCRIPTEDITORWIDGET_H */
