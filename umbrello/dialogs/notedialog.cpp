/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2015                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "notedialog.h"

#include "notewidget.h"
#include "documentationwidget.h"

// kde includes
#include <KLocalizedString>

// qt includes
#include <QFrame>
#include <QVBoxLayout>

/**
 * Constructs an NoteDialog.
 */
NoteDialog::NoteDialog(QWidget * parent, NoteWidget * pNote)
  : SinglePageDialogBase(parent)
{
    setCaption(i18n("Note Documentation"));

    m_pNoteWidget = pNote;
    QFrame *frame = new QFrame(this);
    setMainWidget(frame);
    m_docWidget = new DocumentationWidget(m_pNoteWidget);
    QVBoxLayout *layout = new QVBoxLayout(frame);
    layout->addWidget(m_docWidget, 10);
    setMinimumSize(600, 250);
    m_docWidget->setFocus();
}

/**
 *  Standard destructor.
 */
NoteDialog::~NoteDialog()
{
}

bool NoteDialog::apply()
{
    m_docWidget->apply();
    return true;
}

