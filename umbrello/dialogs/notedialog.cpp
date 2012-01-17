/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "notedialog.h"

// kde includes
#include <klocale.h>
#include <kmessagebox.h>
#include <ktextedit.h>

// qt includes
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QGroupBox>

NoteDialog::NoteDialog( QWidget * parent, NoteWidget * pNote )
  : KDialog(parent)
{
    setCaption( i18n("Note Documentation") );
    setButtons( Help | Ok | Cancel );
    setDefaultButton( Ok );
    setModal( true );
    showButtonSeparator( true );

    m_pNoteWidget = pNote;
    int margin = fontMetrics().height();

    QFrame *frame = new QFrame( this );
    setMainWidget( frame );
    m_docGB = new QGroupBox(i18n("Documentation"), frame);
    QVBoxLayout * mainLayout = new QVBoxLayout(frame);
    mainLayout->addWidget(m_docGB);
    mainLayout->setSpacing(10);
    mainLayout->setMargin(margin);

    QHBoxLayout * docLayout = new QHBoxLayout(m_docGB);
    docLayout->setSpacing(10);
    docLayout->setMargin(margin);

    m_docTE = new KTextEdit( m_docGB );
    m_docTE->setFocus();
    docLayout->addWidget( m_docTE );
    m_docTE->setText( pNote->documentation() );
    setMinimumSize(330, 160);
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
}

NoteDialog::~NoteDialog()
{
}

void NoteDialog::slotOk()
{
    m_pNoteWidget->setDocumentation( m_docTE->toPlainText() );
    accept();
}

#include "notedialog.moc"
