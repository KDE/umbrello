/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "overwritedialog.h"

// kde includes
#include <klocale.h>

// qt includes
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QVBoxLayout>

/**
 * Constructor sets up the dialog, adding checkbox and label.
 */
OverwriteDialog::OverwriteDialog(
        const QString& fileName,
        const QString& outputDirectory,
        bool applyToAllRemaining, QWidget* parent)
  : QDialog(parent)
{
    setWindowTitle(i18n("Destination File Already Exists"));
    setModal(true);

    QVBoxLayout* layout = new QVBoxLayout();
//FIXME KF5    layout->setSpacing(spacingHint());
    layout->setMargin(0);
    setLayout(layout);

    QLabel* dialogueLabel = new QLabel(i18n("The file %1 already exists in %2.\n\nUmbrello can overwrite the file, generate a similar\nfile name or not generate this file.", fileName, outputDirectory));
    layout->addWidget(dialogueLabel);

    m_applyToAllRemaining = new QCheckBox(i18n("&Apply to all remaining files"));
    m_applyToAllRemaining->setChecked(applyToAllRemaining);
    layout->addWidget(m_applyToAllRemaining);

    QDialogButtonBox *buttonBox = new QDialogButtonBox();
    buttonBox->addButton(i18n("&Overwrite"), QDialogButtonBox::AcceptRole);
//FIXME KF5    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
//FIXME KF5    okButton->setDefault(true);
    buttonBox->addButton(i18n("&Generate Similar File Name"), QDialogButtonBox::ApplyRole);
    buttonBox->addButton(i18n("&Do Not Generate File"), QDialogButtonBox::RejectRole);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(slotOk()));
//FIXME KF5    connect(buttonBox, SIGNAL(clicked()), this, SLOT(slotApply()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(slotCancel()));
}

/**
 * Destructor.
 */
OverwriteDialog::~OverwriteDialog()
{
}

/**
 * Overrides standard operation to call QDialog::done(Yes).
 * This is a kludge, see note in class description.
 */
void OverwriteDialog::slotOk()
{
    accept();
}

/**
 * Overrides standard operation to call QDialog::done(No).
 */
void OverwriteDialog::slotApply()
{
//FIXME KF5    done(No);
}

/**
 * Overrides standard operation to call QDialog::done(Cancel).
 */
void OverwriteDialog::slotCancel()
{
    reject();
}

/**
 * @return the value of the Apply To All Remaining Files checkbox
 */
bool OverwriteDialog::applyToAllRemaining()
{
    return m_applyToAllRemaining->isChecked();
}
