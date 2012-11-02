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
 * Handles a mouse move event.
 * Executes base code.
 *
 * @param me The QGraphicsSceneMouseEvent event.
 */
void NoteWidgetController::mouseMoveEvent(QGraphicsSceneMouseEvent *me)
{
    UMLWidgetController::mouseMoveEvent(me);
}

/**
 * Overridden from UMLWidgetController.
 * Handles a mouse release event.
 * Executes base code and then draws the text in the note.
 *
 * @param me The QGraphicsSceneMouseEvent event.
 */
void NoteWidgetController::mouseReleaseEvent(QGraphicsSceneMouseEvent *me)
{
    UMLWidgetController::mouseReleaseEvent(me);
    //TODO why is it needed? drawText is already called in draw,
    //and draw is (well, I think that is) called when the canvas rectangle is resized
    if (m_resized) {
        //m_noteWidget->drawText();
    }
}

/**
 * Overridden from UMLWidgetController.
 * Executes the action for double click in the widget.
 * Shows the dialog to change the text of the note.
 *
 * @param me The QMouseEvent which triggered the double click event.
 */
void NoteWidgetController::doMouseDoubleClick(QGraphicsSceneMouseEvent* me)
{
    Q_UNUSED(me);
    //TODO Copied from old code. What it does?
    // [PORT]
#if 0
    if (m_noteWidget->m_DiagramLink == Uml::id_None) {
        // setup popup menu if not already created.
        if ( !m_noteWidget->m_pMenu )
            m_noteWidget->setupPopupMenu();

        QAction* action = m_noteWidget->m_pMenu->getAction(ListPopupMenu::mt_Rename);
        m_noteWidget->slotMenuSelection(action);
    } else {
        UMLDoc *umldoc = UMLApp::app()->getDocument();
        umldoc->changeCurrentView(m_noteWidget->m_DiagramLink);
    }
#endif
}
