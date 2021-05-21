/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2011 Martin Kuettler <martin.kuettler@gmail.com>
*/

#ifndef IMAGESETTINGSDIALOG_H
#define IMAGESETTINGSDIALOG_H

#include <QDialog>

#include <ui_imagesettings.h>

struct ImageSize
{
    enum {Auto = 0, Pixel = 1, Percent = 2};
    double width;
    double height;
    int widthUnit;
    int heightUnit;
};

class ImageSettingsDialog : public QDialog
{
  Q_OBJECT
  public:
    explicit ImageSettingsDialog(QWidget* parent);

    void setData(const QString& file, const ImageSize& displaySize, const ImageSize& printSize, bool useDisplaySizeForPrinting);

  Q_SIGNALS:
    void dataChanged(const QString& file, const ImageSize& displaySize, const ImageSize& printSize, bool useDisplaySizeForPrinting);

  private Q_SLOTS:
    void sendChangesAndClose();
    void sendChanges();

    void openDialog();
    void updatePreview();
    void updateInputWidgets();
    void updatePrintingGroup(int b);

  private:
    QList<QString> m_unitNames;
    Ui_ImageSettingsBase m_ui;

};

#endif //IMAGESETTINGSDIALOG_H
