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

#ifndef MATHEMATIKPART_H
#define MATHEMATIKPART_H

#include <kparts/part.h>
#include <lib/session.h>

class QWidget;
class QPainter;
class KUrl;
class Worksheet;
class KAboutData;
class KAction;
class KToggleAction;

/**
 * This is a "Part".  It that does all the real work in a KPart
 * application.
 *
 * @short Main Part
 * @author Alexander Rieder <alexanderrieder@gmail.com>
 */
class MathematiKPart : public KParts::ReadWritePart
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    MathematiKPart(QWidget *parentWidget,QObject *parent, const QStringList &args);

    /**
     * Destructor
     */
    virtual ~MathematiKPart();

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

protected:
    /**
     * This must be implemented by each part
     */
    virtual bool openFile();

    /**
     * This must be implemented by each read-write part
     */
    virtual bool saveFile();

    void loadAssistants();

protected slots:
    void fileSaveAs();
    void evaluateOrInterrupt();
    void restartBackend();
    void enableTypesetting(bool enable);
    void showBackendHelp();
    
    void worksheetStatusChanged(MathematiK::Session::Status stauts);
    void worksheetSessionChanged();
    void initialized();

    void updateCaption();

    void runAssistant();

private:
    Worksheet *m_worksheet;

    KAction* m_evaluate;
    KAction* m_save;
    KToggleAction* m_typeset;
    KAction* m_showBackendHelp;
};

#endif // MATHEMATIKPART_H
