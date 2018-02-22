/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2018                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "notedialog.h"
#include "notewidget.h"

/**
 * Constructs an NoteDialog.
 */
NoteDialog::NoteDialog(QWidget * parent, NoteWidget * widget)
  : MultiPageDialogBase(parent),
    m_widget(widget)
{
    setupPages();
    connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));
    connect(this, SIGNAL(applyClicked()), this, SLOT(slotApply()));
}

void NoteDialog::setupPages()
{
    setupGeneralPage(m_widget);
    setupStylePage(m_widget);
    setupFontPage(m_widget);
}

void NoteDialog::slotOk()
{
    slotApply();
}

void NoteDialog::slotApply()
{
    if (!apply())
        reject();
    else
        accept();
}

bool NoteDialog::apply()
{
    MultiPageDialogBase::apply();
    if (m_widget) {
        applyFontPage(m_widget);
        QString key = QLatin1String("Diagram:");
        QString str = m_widget->documentation();
        if (!str.startsWith(key)) {
            m_widget->setDiagramLink(QString());
            return false;
        }
        QString diagramName = str.remove(key).trimmed();
        m_widget->setDiagramLink(diagramName);
    }
    return true;
}
