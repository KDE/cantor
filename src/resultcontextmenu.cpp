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

#include "resultcontextmenu.h"

#include "lib/result.h"
#include "lib/latexresult.h"
#include "lib/animationresult.h"
#include "worksheetentry.h"

#include <qmovie.h>

#include <kfiledialog.h>
#include <klocale.h>
#include <kdebug.h>

ResultContextMenu::ResultContextMenu(WorksheetEntry* entry, QWidget* parent) : KMenu(parent)
{
    setTitle(i18n("Result"));
    m_entry=entry;

    addGeneralActions();
    addTypeSpecificActions();
}

ResultContextMenu::~ResultContextMenu()
{

}

void ResultContextMenu::addGeneralActions()
{
    QAction* saveAction=addAction(i18n("Save result"));
    connect(saveAction, SIGNAL(triggered()), this, SLOT(saveResult()));
}

void ResultContextMenu::addTypeSpecificActions()
{
    switch(result()->type())
    {
        case Cantor::LatexResult::Type:
        {
            QAction* showCodeAction=0;
            Cantor::LatexResult* lr=dynamic_cast<Cantor::LatexResult*>(result());
            if(lr->isCodeShown())
                showCodeAction=addAction(i18n("Show Rendered"));
            else
                showCodeAction=addAction(i18n("Show Code"));

            connect(showCodeAction, SIGNAL(triggered()), this, SLOT(latexToggleShowCode()));
            break;
        }
        case Cantor::AnimationResult::Type:
        {
            Cantor::AnimationResult* ar=dynamic_cast<Cantor::AnimationResult*>(result());
            QMovie* movie=static_cast<QMovie*>(ar->data().value<QObject*>());
            if(!movie)
                break;

            QAction* startStopAction=0;
            if(movie->state()==QMovie::Running)
                startStopAction=addAction(i18n("Pause Animation"));
            else
                startStopAction=addAction(i18n("Start Animation"));

            connect(startStopAction, SIGNAL(triggered()), this, SLOT(animationPause()));

            QAction* restartAction=addAction(i18n("Restart Animation"));
            connect(restartAction, SIGNAL(triggered()), this, SLOT(animationRestart()));
        }
    }
}

WorksheetEntry* ResultContextMenu::entry()
{
    return m_entry;
}

Cantor::Result* ResultContextMenu::result()
{
    return m_entry->expression()->result();
}


void ResultContextMenu::saveResult()
{
    const QString& filename=KFileDialog::getSaveFileName(KUrl(), result()->mimeType(), this);
    kDebug()<<"saving result to "<<filename;
    result()->save(filename);
}

void ResultContextMenu::latexToggleShowCode()
{
     Cantor::LatexResult* lr=dynamic_cast<Cantor::LatexResult*>(result());
     if(lr->isCodeShown())
         lr->showRendered();
     else
         lr->showCode();

     entry()->updateResult();
}

void ResultContextMenu::animationPause()
{
    Cantor::AnimationResult* ar=dynamic_cast<Cantor::AnimationResult*>(result());
    QMovie* movie=static_cast<QMovie*>(ar->data().value<QObject*>());
    if(!movie)
        return;

    if(movie->state()==QMovie::Running)
        movie->setPaused(true);
    else
        movie->start();
}

void ResultContextMenu::animationRestart()
{
   Cantor::AnimationResult* ar=dynamic_cast<Cantor::AnimationResult*>(result());
    QMovie* movie=static_cast<QMovie*>(ar->data().value<QObject*>());
    if(!movie)
        return;

    movie->stop();
    movie->start();
}
