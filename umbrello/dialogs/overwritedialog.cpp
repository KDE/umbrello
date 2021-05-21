/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
  : SinglePageDialogBase(parent, true)
{
    setCaption(i18n("Destination File Already Exists"));
    setButtonText(Ok, i18n("&Overwrite"));
    setButtonText(Apply, i18n("&Generate Similar File Name"));
    setButtonText(Cancel, i18n("&Do Not Generate File"));
    setModal(true);

    QFrame *frame = new QFrame(this);
    QVBoxLayout* layout = new QVBoxLayout(frame);
    layout->setMargin(0);

    QLabel* dialogueLabel = new QLabel(i18n("The file %1 already exists in %2.\n\nUmbrello can overwrite the file, generate a similar\nfile name or not generate this file.", fileName, outputDirectory));
    layout->addWidget(dialogueLabel);

    m_applyToAllRemaining = new QCheckBox(i18n("&Apply to all remaining files"));
    m_applyToAllRemaining->setChecked(applyToAllRemaining);
    layout->addWidget(m_applyToAllRemaining);
    setMainWidget(frame);
}

/**
 * Destructor.
 */
OverwriteDialog::~OverwriteDialog()
{
}

/**
 * @return the value of the Apply To All Remaining Files checkbox
 */
bool OverwriteDialog::applyToAllRemaining()
{
    return m_applyToAllRemaining->isChecked();
}
