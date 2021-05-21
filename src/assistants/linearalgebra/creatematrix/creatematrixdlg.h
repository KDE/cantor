/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _CREATEMATRIXDLG_H
#define _CREATEMATRIXDLG_H

#include <QDialog>
#include "ui_creatematrixdlg.h"

class CreateMatrixDlg : public QDialog
{
  Q_OBJECT
  public:
    explicit CreateMatrixDlg( QWidget* parent);
    ~CreateMatrixDlg() override;

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
