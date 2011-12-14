/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2010                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// app includes
#include "notewidgetcontroller.h"
#include "notewidget.h"
#include "uml.h"
#include "umldoc.h"
#include "listpopupmenu.h"

/**
 * Constructor for NoteWidgetController.
 *
 * @param noteWidget The NoteWidget which uses the controller.
 */
NoteWidgetController::NoteWidgetController(NoteWidget *noteWidget)
  : UMLWidgetController(noteWidget)
{
    m_noteWidget = noteWidget;
}

/**
 * Destructor for NoteWidgetController.
 */
NoteWidgetController::~NoteWidgetController()
{
}

/**
 * Overridden from UMLWidgetController.
 * Executes the action for double click in the widget.
 * Shows the dialog to change the text of the note.
 *
 * @param me The QMouseEvent which triggered the double click event.
 */
void NoteWidgetController::doMouseDoubleClick(QMouseEvent* me)
{
    Q_UNUSED(me);
    //TODO Copied from old code. What it does?
    if (m_noteWidget->m_diagramLink == Uml::id_None) {
        // setup popup menu if not already created.
        if ( !m_noteWidget->m_pMenu )
            m_noteWidget->setupPopupMenu();

        QAction* action = m_noteWidget->m_pMenu->getAction(ListPopupMenu::mt_Rename);
        m_noteWidget->slotMenuSelection(action);
    } else {
        UMLDoc *umldoc = UMLApp::app()->document();
        umldoc->changeCurrentView(m_noteWidget->m_diagramLink);
    }
}
