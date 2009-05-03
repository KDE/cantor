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
    
    void worksheetStatusChanged(MathematiK::Session::Status stauts);
    void worksheetSessionChanged();
    void initialized();

    void updateCaption();

    void runAssistant();

private:
    Worksheet *m_worksheet;

    KAction* m_evaluate;
    KAction *m_save;
    KToggleAction* m_typeset;
};

#endif // MATHEMATIKPART_H
