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

#ifndef _CREATEMATRIXDLG_H
#define _CREATEMATRIXDLG_H

#include <kdialog.h>
#include "ui_creatematrixdlg.h"

class CreateMatrixDlg : public KDialog
{
  Q_OBJECT
  public:
    CreateMatrixDlg( QWidget* parent);
    ~CreateMatrixDlg();

    int numRows();
    int numCols();

    QString value(int i,int j);

  public Q_SLOTS:
    void changeNumRows(int rows);
    void changeNumCols(int cols);

  private:
    Ui::CreateMatrixAssistantBase *m_base;
};

#endif /* _CREATEMATRIXDLG_H */
