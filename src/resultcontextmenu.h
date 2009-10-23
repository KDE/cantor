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

#ifndef _RESULTCONTEXTMENU_H
#define _RESULTCONTEXTMENU_H

#include <kmenu.h>

class WorksheetEntry;
namespace Cantor{
    class Result;
}

/**
 * this is the Menu shown when Right clicking on a Result.
 *  It offers different options depending on the Type of the
 *  result.
 **/
class ResultContextMenu : public KMenu
{
  Q_OBJECT
  public:
    ResultContextMenu( WorksheetEntry* entry, QWidget* parent);
    ~ResultContextMenu();

    WorksheetEntry* entry();
    Cantor::Result* result();

  public slots:
    void saveResult();
    void latexToggleShowCode();
    void animationPause();
    void animationRestart();

  private:
    void addGeneralActions();
    void addTypeSpecificActions();

  private:
    WorksheetEntry* m_entry;
    Cantor::Result* m_result;

};

#endif /* _RESULTCONTEXTMENU_H */
