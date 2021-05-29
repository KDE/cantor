/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "creatematrixdlg.h"
#include <QPushButton>

CreateMatrixDlg::CreateMatrixDlg(QWidget* parent) : QDialog(parent)
{
    m_base=new Ui::CreateMatrixAssistantBase;
    QWidget* mainW=new QWidget(this);
    m_base->setupUi(mainW);

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);
    layout->addWidget(mainW);

    m_base->buttonBox->button(QDialogButtonBox::Ok)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOkButton));
    m_base->buttonBox->button(QDialogButtonBox::Cancel)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));
    connect(m_base->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_base->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    connect(m_base->rows, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &CreateMatrixDlg::changeNumRows);
    connect(m_base->columns, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &CreateMatrixDlg::changeNumCols);
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
        return QLatin1String("0");
}


