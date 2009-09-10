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

#include "creatematrixdlg.h"

CreateMatrixDlg::CreateMatrixDlg(QWidget* parent) : KDialog(parent)
{
    m_base=new Ui::CreateMatrixAssistantBase;
    QWidget* mainW=new QWidget(this);
    m_base->setupUi(mainW);
    setMainWidget(mainW);

    connect(m_base->rows, SIGNAL(valueChanged(int)), this, SLOT(changeNumRows(int)));
    connect(m_base->columns, SIGNAL(valueChanged(int)), this, SLOT(changeNumCols(int)));
}

CreateMatrixDlg::~CreateMatrixDlg()
{
    delete m_base;
}

void CreateMatrixDlg::changeNumRows(int rows)
{
    m_base->values->setRowCount(rows);
}

void CreateMatrixDlg::changeNumCols(int cols)
{
    m_base->values->setColumnCount(cols);
}

int CreateMatrixDlg::numRows()
{
    return m_base->rows->value();
}

int CreateMatrixDlg::numCols()
{
    return m_base->columns->value();
}

QString CreateMatrixDlg::value(int i, int j)
{
    const QTableWidgetItem* item=m_base->values->item(i, j);
    if(item)
        return item->text();
    else
        return "0";
}

#include "creatematrixdlg.moc"
