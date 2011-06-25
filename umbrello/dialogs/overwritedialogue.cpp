/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "overwritedialogue.h"

// qt/kde includes
#include <QtGui/QVBoxLayout>
#include <QtGui/QCheckBox>
#include <QtGui/QLayout>
#include <QtGui/QLabel>

#include <klocale.h>


/**
 * Constructor sets up the dialog, adding checkbox and label.
 */
OverwriteDialogue::OverwriteDialogue(
        const QString& fileName,
        const QString& outputDirectory,
        bool applyToAllRemaining, QWidget* parent)
  : KDialog(parent)
{
    setCaption( i18n("Destination File Already Exists") );
    setButtons( Help | Ok | Cancel );
    setDefaultButton( Yes );
    setModal( true );
    showButtonSeparator( true );

    QFrame *frame = new QFrame( this );
    setMainWidget( frame );

    QVBoxLayout* layout = new QVBoxLayout( frame );
    layout->setSpacing( spacingHint() );
    layout->setMargin( 0 );

    QLabel* dialogueLabel = new QLabel(i18n("The file %1 already exists in %2.\n\nUmbrello can overwrite the file, generate a similar\nfile name or not generate this file.", fileName, outputDirectory), frame );
    layout->addWidget(dialogueLabel);

    m_applyToAllRemaining = new QCheckBox( i18n("&Apply to all remaining files"), frame );
    m_applyToAllRemaining->setChecked(applyToAllRemaining);
    layout->addWidget(m_applyToAllRemaining);

    setButtonText(KDialog::Ok, i18n("&Overwrite"));
    setButtonText(KDialog::Apply, i18n("&Generate Similar File Name"));
    setButtonText(KDialog::Cancel, i18n("&Do Not Generate File"));
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    connect(this,SIGNAL(applyClicked()),this,SLOT(slotApply()));
    connect(this,SIGNAL(cancelClicked()),this,SLOT(slotCancel()));
}

/**
 * Destructor.
 */
OverwriteDialogue::~OverwriteDialogue()
{
}

/**
 * Overrides standard operation to call QDialog::done(Yes).
 * This is a kludge, see note in class description.
 */
void OverwriteDialogue::slotOk()
{
    done(Yes);
}

/**
 * Overrides standard operation to call QDialog::done(No).
 */
void OverwriteDialogue::slotApply()
{
    done(No);
}

/**
 * Overrides standard operation to call QDialog::done(Cancel).
 */
void OverwriteDialogue::slotCancel()
{
    done(Cancel);
}

/**
 * @return the value of the Apply To All Remaining Files checkbox
 */
bool OverwriteDialogue::applyToAllRemaining()
{
    return m_applyToAllRemaining->isChecked();
}

#include "overwritedialogue.moc"
