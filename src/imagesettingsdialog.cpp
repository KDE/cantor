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
    Copyright (C) 2011 Martin Kuettler <martin.kuettler@gmail.com>
 */

#include "imagesettingsdialog.h"
#include "qimagereader.h"
#include "qfiledialog.h"
#include "kurlcompletion.h"

ImageSettingsDialog::ImageSettingsDialog(QWidget* parent) : KDialog(parent)
{
    QWidget *w = new QWidget(this);
    m_ui.setupUi(w);
    setMainWidget(w);
    this->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );

    m_units << "(auto)" << "px" << "%";
    m_unitNames << i18n("(auto)") << i18n("px") << i18n("%");

    m_ui.displayWidthCombo->addItems(m_unitNames);
    m_ui.displayHeightCombo->addItems(m_unitNames);
    m_ui.printWidthCombo->addItems(m_unitNames);
    m_ui.printHeightCombo->addItems(m_unitNames);

    KUrlCompletion* completer = new KUrlCompletion(KUrlCompletion::FileCompletion);
    completer->setCompletionMode(KGlobalSettings::CompletionMan);
    m_ui.pathEdit->setCompletionObject(completer);
    m_ui.pathEdit->setAutoDeleteCompletionObject( true );

    m_ui.displayWidthInput->setMinimum(0);
    m_ui.displayHeightInput->setMinimum(0);
    m_ui.printWidthInput->setMinimum(0);
    m_ui.printHeightInput->setMinimum(0);
    m_ui.displayWidthInput->setSingleStep(1);
    m_ui.displayHeightInput->setSingleStep(1);
    m_ui.printWidthInput->setSingleStep(1);
    m_ui.printHeightInput->setSingleStep(1);

    connect(this, SIGNAL(okClicked()), this, SLOT(sendChangesAndClose()));
    connect(this, SIGNAL(applyClicked()), this, SLOT(sendChanges()));
    connect(this, SIGNAL(cancelClicked()), this, SLOT(close()));

    connect(m_ui.openDialogButton, SIGNAL(clicked()), this, SLOT(openDialog()));
    //connect(m_fileDialog, SIGNAL(accepted()), this, SLOT(updatePath()));

    connect(m_ui.pathEdit, SIGNAL(editingFinished()), this, SLOT(updatePreview()));

    connect(m_ui.displayWidthCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateInputWidgets()));
    connect(m_ui.displayHeightCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateInputWidgets()));
    connect(m_ui.printWidthCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateInputWidgets()));
    connect(m_ui.printHeightCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateInputWidgets()));

    connect(m_ui.useDisplaySize, SIGNAL(stateChanged(int)), this, SLOT(updatePrintingGroup(int)));
}

ImageSettingsDialog::~ImageSettingsDialog()
{

}

void ImageSettingsDialog::setData(const QString& file, const ImageSize& displaySize, const ImageSize& printSize, bool useDisplaySizeForPrinting) 
{
    m_ui.pathEdit->setText(file);
    if (displaySize.width >= 0)
	m_ui.displayWidthInput->setValue(displaySize.width);
    if (displaySize.height >= 0)
	m_ui.displayHeightInput->setValue(displaySize.height);
    if (printSize.width >= 0)
	m_ui.printWidthInput->setValue(printSize.width);
    if (printSize.height >= 0)
	m_ui.printHeightInput->setValue(printSize.height);
    if (displaySize.widthUnit.isEmpty())
	m_ui.displayWidthCombo->setCurrentIndex(0);
    else
	m_ui.displayWidthCombo->setCurrentIndex(m_units.indexOf(displaySize.widthUnit));
    if (displaySize.heightUnit.isEmpty())
	m_ui.displayHeightCombo->setCurrentIndex(0);
    else
	m_ui.displayHeightCombo->setCurrentIndex(m_units.indexOf(displaySize.heightUnit));
    if (printSize.widthUnit.isEmpty())
	m_ui.printWidthCombo->setCurrentIndex(0);
    else
	m_ui.printWidthCombo->setCurrentIndex(m_units.indexOf(printSize.widthUnit));
    if (printSize.heightUnit.isEmpty())
	m_ui.printHeightCombo->setCurrentIndex(0);
    else
	m_ui.printHeightCombo->setCurrentIndex(m_units.indexOf(printSize.heightUnit));
    if (useDisplaySizeForPrinting)
	m_ui.useDisplaySize->setCheckState(Qt::Checked);
    else
	m_ui.useDisplaySize->setCheckState(Qt::Unchecked);

    updatePreview();
    updatePrintingGroup(useDisplaySizeForPrinting);
    //updateInputWidgets();

}

void ImageSettingsDialog::sendChangesAndClose()
{
    sendChanges();
    close();
}

void ImageSettingsDialog::sendChanges()
{
    ImageSize displaySize, printSize;
    displaySize.width = m_ui.displayWidthInput->value();
    displaySize.height = m_ui.displayHeightInput->value();
    displaySize.widthUnit = m_units[m_ui.displayWidthCombo->currentIndex()];
    displaySize.heightUnit = m_units[m_ui.displayHeightCombo->currentIndex()];
    printSize.width = m_ui.printWidthInput->value();
    printSize.height = m_ui.printHeightInput->value();
    printSize.widthUnit = m_units[m_ui.printWidthCombo->currentIndex()];
    printSize.heightUnit = m_units[m_ui.printHeightCombo->currentIndex()];

    emit dataChanged
	(m_ui.pathEdit->text(), displaySize, printSize,
	 (m_ui.useDisplaySize->checkState() == Qt::Checked));
}

void ImageSettingsDialog::openDialog()
{
    QList<QByteArray> formats = QImageReader::supportedImageFormats();
    QString formatString = "Images(";
    foreach(QByteArray format, formats)
    {
	formatString += "*." + QString(format).toLower() + " ";
    }
    formatString += ")";
    QString file = QFileDialog::getOpenFileName(this, i18n("Open image file"), m_ui.pathEdit->text(), formatString);
    if (!file.isEmpty())
    {
	m_ui.pathEdit->setText(file);
	updatePreview();
    }
}

void ImageSettingsDialog::updatePreview()
{
    m_ui.imagePreview->showPreview(KUrl(m_ui.pathEdit->text()));
}

void ImageSettingsDialog::updateInputWidgets()
{
    if (m_ui.displayWidthCombo->currentIndex() == 0)
	m_ui.displayWidthInput->setEnabled(false);
    else
	m_ui.displayWidthInput->setEnabled(true);

    if (m_ui.displayHeightCombo->currentIndex() == 0)
	m_ui.displayHeightInput->setEnabled(false);
    else
	m_ui.displayHeightInput->setEnabled(true);
	
    if (m_ui.printWidthCombo->currentIndex() == 0 || !m_ui.printWidthCombo->isEnabled())
	m_ui.printWidthInput->setEnabled(false);
    else
	m_ui.printWidthInput->setEnabled(true);

    if (m_ui.printHeightCombo->currentIndex() == 0 || !m_ui.printHeightCombo->isEnabled())
	m_ui.printHeightInput->setEnabled(false);
    else
	m_ui.printHeightInput->setEnabled(true);
}

void ImageSettingsDialog::updatePrintingGroup(int b)
{
    
    m_ui.printWidthCombo->setEnabled(!b);
    m_ui.printHeightCombo->setEnabled(!b);

    updateInputWidgets();
}
