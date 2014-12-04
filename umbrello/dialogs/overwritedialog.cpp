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
#include <KLocalizedString>

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
OverwriteDialog::OverwriteDialog(const QString& fileName, const QString& outputDirectory,
                                 bool applyToAllRemaining, QWidget* parent)
  : QDialog(parent)
{
    setWindowTitle(i18n("Destination File Already Exists"));
    setModal(true);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setMargin(0);
    setLayout(layout);

    QLabel* dialogueLabel = new QLabel(i18n("The file %1 already exists in %2.\n\nUmbrello can overwrite the file, generate a similar\nfile name or not generate this file.", fileName, outputDirectory));
    layout->addWidget(dialogueLabel);

    m_applyToAllRemaining = new QCheckBox(i18n("&Apply to all remaining files"));
    m_applyToAllRemaining->setChecked(applyToAllRemaining);
    layout->addWidget(m_applyToAllRemaining);

    QDialogButtonBox *buttonBox = new QDialogButtonBox();
    QPushButton* okayBtn = buttonBox->addButton(i18n("&Overwrite"), QDialogButtonBox::AcceptRole);
    QPushButton* applyBtn = buttonBox->addButton(i18n("&Generate Similar File Name"), QDialogButtonBox::ApplyRole);
    QPushButton* cancelBtn = buttonBox->addButton(i18n("&Do Not Generate File"), QDialogButtonBox::RejectRole);
    layout->addWidget(buttonBox);
    connect(okayBtn, SIGNAL(clicked()), this, SLOT(slotOk()));
    connect(applyBtn, SIGNAL(clicked()), this, SLOT(slotApply()));
    connect(cancelBtn, SIGNAL(clicked()), this, SLOT(slotCancel()));
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
    done(QDialogButtonBox::Yes);
}

/**
 * Overrides standard operation to call QDialog::done(No).
 */
void OverwriteDialog::slotApply()
{
    done(QDialogButtonBox::No);
}

/**
 * Overrides standard operation to call QDialog::done(Cancel).
 */
void OverwriteDialog::slotCancel()
{
    done(QDialogButtonBox::Cancel);
}

/**
 * @return the value of the Apply To All Remaining Files checkbox
 */
bool OverwriteDialog::applyToAllRemaining()
{
    return m_applyToAllRemaining->isChecked();
}
