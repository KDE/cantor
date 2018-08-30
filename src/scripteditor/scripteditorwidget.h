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
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
 */

#ifndef _SCRIPTEDITORWIDGET_H
#define _SCRIPTEDITORWIDGET_H

#include <KXmlGuiWindow>

class QTemporaryFile;
class QGridLayout;
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
    ~ScriptEditorWidget() override = default;
    void open(QUrl url);

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
    KTextEditor::View* m_editor;
    KTextEditor::Document* m_script;
    QTemporaryFile* m_tmpFile;
};

#endif /* _SCRIPTEDITORWIDGET_H */
