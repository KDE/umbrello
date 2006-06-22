/*
 *  copyright (C) 2002-2004
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qcheckbox.h>
#include <qlayout.h>
#include <qlabel.h>
//Added by qt3to4:
#include <QVBoxLayout>

#include <klocale.h>
#include <kdebug.h>

#include "overwritedialogue.h"

OverwriteDialogue::OverwriteDialogue(QString fileName, QString outputDirectory, bool applyToAllRemaining, QWidget* parent, const char* name)
    :KDialog(parent) {
    setCaption( i18n("Destination File Already Exists") );
    setButtons( Help | Ok | Cancel );
    setDefaultButton( Yes );
    setModal( true );
    enableButtonSeparator( true );

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
}

OverwriteDialogue::~OverwriteDialogue() {
}

void OverwriteDialogue::slotOk() {
    done(Yes);
}

void OverwriteDialogue::slotApply() {
    done(No);
}

void OverwriteDialogue::slotCancel() {
    done(Cancel);
}

bool OverwriteDialogue::applyToAllRemaining() {
    return m_applyToAllRemaining->isChecked();
}

#include "overwritedialogue.moc"
