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

#include "mathematik_part.h"

#include "mathematik_part.moc"

#include <kaction.h>
#include <kactioncollection.h>
#include <kcomponentdata.h>
#include <kfiledialog.h>
#include <kparts/genericfactory.h>
#include <kstandardaction.h>
#include <kzip.h>
#include <ktoggleaction.h>
#include <kservice.h>
#include <kservicetypetrader.h>

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QTextEdit>

#include "worksheet.h"
#include "lib/backend.h"
#include "lib/assistant.h"

#include "helpextension.h"

typedef KParts::GenericFactory<MathematiKPart> MathematiKPartFactory;
K_EXPORT_COMPONENT_FACTORY( libmathematikpart, MathematiKPartFactory )

MathematiKPart::MathematiKPart( QWidget *parentWidget, QObject *parent, const QStringList & args ): KParts::ReadWritePart(parent)
{
    // we need an instance
    setComponentData( MathematiKPartFactory::componentData() );

    HelpExtension* h=new HelpExtension(this);

    kDebug()<<"Created a MathematiKPart";

    MathematiK::Backend* b=MathematiK::Backend::createBackend(args.first(), this);
    kDebug()<<"Backend "<<b->name()<<" offers extensions: "<<b->extensions();

    m_worksheet=new Worksheet(b, parentWidget);
    connect(m_worksheet, SIGNAL(modified()), this, SLOT(setModified()));
    connect(m_worksheet, SIGNAL(sessionChanged()), this, SLOT(worksheetSessionChanged()));
    connect(m_worksheet, SIGNAL(showHelp(const QString&)), h, SIGNAL(showHelp(const QString&)));
    m_worksheet->setEnabled(false); //disable input until the session has sucessfully logged in and emits the ready signal
    worksheetSessionChanged();

    // notify the part that this is our internal widget
    setWidget(m_worksheet);

    // create our actions
    KStandardAction::saveAs(this, SLOT(fileSaveAs()), actionCollection());
    m_save = KStandardAction::save(this, SLOT(save()), actionCollection());

    m_evaluate=new KAction(i18n("Evaluate Worksheet"), actionCollection());
    actionCollection()->addAction("evaluate", m_evaluate);
    m_evaluate->setIcon(KIcon("system-run"));
    connect(m_evaluate, SIGNAL(triggered()), this, SLOT(evaluateOrInterrupt()));

    m_typeset=new KToggleAction(i18n("Typeset using LaTeX"), actionCollection());
    m_typeset->setChecked(true);
    actionCollection()->addAction("enable_typesetting", m_typeset);
    connect(m_typeset, SIGNAL(toggled(bool)), this, SLOT(enableTypesetting(bool)));

    KAction* restart=new KAction(i18n("Restart Backend"), actionCollection());
    actionCollection()->addAction("restart_backend", restart);
    restart->setIcon(KIcon("system-restart"));
    connect(restart, SIGNAL(triggered()), this, SLOT(restartBackend()));

    // set our XML-UI resource file
    setXMLFile("mathematik_part.rc");

    // we are read-write by default
    setReadWrite(true);

    // we are not modified since we haven't done anything yet
    setModified(false);

}

MathematiKPart::~MathematiKPart()
{
}

void MathematiKPart::setReadWrite(bool rw)
{
    // notify your internal widget of the read-write state
    m_worksheet->setReadOnly(!rw);

    ReadWritePart::setReadWrite(rw);
}

void MathematiKPart::setModified(bool modified)
{
    // get a handle on our Save action and make sure it is valid
    if (!m_save)
        return;

    // if so, we either enable or disable it based on the current
    // state
    if (modified)
        m_save->setEnabled(true);
    else
        m_save->setEnabled(false);

    // in any event, we want our parent to do it's thing
    ReadWritePart::setModified(modified);
}

KAboutData *MathematiKPart::createAboutData()
{
    // the non-i18n name here must be the same as the directory in
    // which the part's rc file is installed ('partrcdir' in the
    // Makefile)
    KAboutData *aboutData = new KAboutData("mathematikpart", 0, ki18n("MathematiKPart"), "0.1");
    aboutData->addAuthor(ki18n("Alexander Rieder"), KLocalizedString(), "alexanderrieder@gmail.com");
    return aboutData;
}

bool MathematiKPart::openFile()
{
    m_worksheet->load(localFilePath());

    // just for fun, set the status bar
    //emit setStatusBarText( m_url.prettyUrl() );

    updateCaption();

    return true;
}

bool MathematiKPart::saveFile()
{
    // if we aren't read-write, return immediately
    if (isReadWrite() == false)
        return false;

    kDebug()<<"saving to: "<<url();
    if (url().isEmpty())
        fileSaveAs();
    else
        m_worksheet->save( localFilePath() );
    setModified(false);

    return true;
}

void MathematiKPart::fileSaveAs()
{
    // this slot is called whenever the File->Save As menu is selected,
    QString file_name = KFileDialog::getSaveFileName(KUrl(), "*.mws", widget());
    if (file_name.isEmpty() == false)
        saveAs(file_name);

    updateCaption();
}

void MathematiKPart::evaluateOrInterrupt()
{
    kDebug()<<"evalorinterrupt";
    if(m_worksheet->isRunning())
        m_worksheet->interrupt();
    else
        m_worksheet->evaluate();
}

void MathematiKPart::restartBackend()
{
    m_worksheet->session()->logout();
    m_worksheet->session()->login();
}

void MathematiKPart::worksheetStatusChanged(MathematiK::Session::Status status)
{
    if(status==MathematiK::Session::Running)
    {
        m_evaluate->setText(i18n("Interrupt"));
        m_evaluate->setIcon(KIcon("dialog-close"));
    }else
    {
        m_evaluate->setText(i18n("Evaluate Worksheet"));
        m_evaluate->setIcon(KIcon("system-run"));
    }
}

void MathematiKPart::worksheetSessionChanged()
{
    connect(m_worksheet->session(), SIGNAL(statusChanged(MathematiK::Session::Status)), this, SLOT(worksheetStatusChanged(MathematiK::Session::Status)));
    connect(m_worksheet->session(), SIGNAL(ready()),this, SLOT(initialized()));

    loadAssistants();
}

void MathematiKPart::initialized()
{
    m_worksheet->setEnabled(true);
    emit setStatusBarText(i18n("Initialization complete"));

    updateCaption();
}

void MathematiKPart::enableTypesetting(bool enable)
{
    m_worksheet->session()->setTypesettingEnabled(enable);
}

Worksheet* MathematiKPart::worksheet()
{
    return m_worksheet;
}

void MathematiKPart::updateCaption()
{
    QString filename=url().fileName();
    if (filename.isEmpty())
        filename=i18n("Unnamed");

    emit setWindowCaption(i18n("%1: %2", m_worksheet->session()->backend()->name(), filename));
}

void MathematiKPart::loadAssistants()
{
    kDebug()<<"loading assistants...";

    KService::List services;
    KServiceTypeTrader* trader = KServiceTypeTrader::self();

    services = trader->query("MathematiK/Assistant");

    foreach (KService::Ptr service,   services)
    {
        QString error;

        kDebug()<<"found service"<<service->name();
        MathematiK::Assistant* assistant=service->createInstance<MathematiK::Assistant>(this,  QVariantList(),   &error);
        if (assistant==0)
        {
            kDebug()<<"error loading assistant"<<service->name()<<":  "<<error;
            continue;
        }

        kDebug()<<"created it";
        MathematiK::Backend* backend=worksheet()->session()->backend();
        KPluginInfo info(service);
        assistant->setPluginInfo(info);
        assistant->setBackend(backend);

        kDebug()<<"plugin "<<service->name()<<" requires "<<assistant->requiredExtensions();
        bool supported=true;
        foreach(QString req, assistant->requiredExtensions())
            supported=supported && backend->extensions().contains(req);
        kDebug()<<"plugin "<<service->name()<<" is "<<(supported ? "":" not ")<<" supported by "<<backend->name();

        if(supported)
        {
            assistant->initActions();
            //createGui(assistant);
            connect(assistant, SIGNAL(requested()), this, SLOT(runAssistant()));
        }else
        {
            assistant->deleteLater();
        }
    }


}

void MathematiKPart::runAssistant()
{
    MathematiK::Assistant* a=qobject_cast<MathematiK::Assistant*>(sender());
    QStringList cmds=a->run(widget());
    kDebug()<<cmds;
    m_worksheet->appendEntry(cmds.join("\n"));
}

