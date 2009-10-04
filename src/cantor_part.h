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

#ifndef CANTORPART_H
#define CANTORPART_H

#include <kparts/part.h>
#include <lib/session.h>

class QWidget;
class Worksheet;
class ScriptEditorWidget;
class KAboutData;
class KAction;
class KToggleAction;
class KProgressDialog;

/**
 * This is a "Part".  It that does all the real work in a KPart
 * application.
 *
 * @short Main Part
 * @author Alexander Rieder <alexanderrieder@gmail.com>
 */
class CantorPart : public KParts::ReadWritePart
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    CantorPart(QWidget *parentWidget,QObject *parent, const QStringList &args);

    /**
     * Destructor
     */
    virtual ~CantorPart();

    /**
     * This is a virtual function inherited from KParts::ReadWritePart.
     * A shell will use this to inform this Part if it should act
     * read-only
     */
    virtual void setReadWrite(bool rw);

    /**
     * Reimplemented to disable and enable Save action
     */
    virtual void setModified(bool modified);

    static KAboutData *createAboutData();

    Worksheet* worksheet();

signals:
    void setCaption(const QString& caption);
    void showHelp(const QString& help);

protected:
    /**
     * This must be implemented by each part
     */
    virtual bool openFile();

    /**
     * This must be implemented by each read-write part
     */
    virtual bool saveFile();

    /**
     * Called when this part becomes the active one,
     * or if it looses activity
     **/
    void guiActivateEvent( KParts::GUIActivateEvent * event );


    void loadAssistants();
    void adjustGuiToSession();

protected slots:
    void fileSaveAs();
    void evaluateOrInterrupt();
    void restartBackend();
    void enableTypesetting(bool enable);
    void showBackendHelp();
    void print();
    
    void worksheetStatusChanged(Cantor::Session::Status stauts);
    void showSessionError(const QString& error);
    void worksheetSessionChanged();
    void initialized();
    void updateCaption();

    void runAssistant();  
    void publishWorksheet();

    void showScriptEditor(bool show);
    void runScript(const QString& file);
private:
    Worksheet *m_worksheet;
    ScriptEditorWidget* m_scriptEditor;

    KProgressDialog* m_initProgressDlg;
    KAction* m_evaluate;
    KAction* m_save;
    KToggleAction* m_typeset;
    KToggleAction* m_highlight;
    KToggleAction* m_tabcompletion;
    KToggleAction* m_exprNumbering;
    KAction* m_showBackendHelp;
};

#endif // CANTORPART_H
