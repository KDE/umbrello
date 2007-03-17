/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// app includes
#include "notewidgetcontroller.h"
#include "notewidget.h"
#include "uml.h"
#include "umldoc.h"
#include "listpopupmenu.h"

NoteWidgetController::NoteWidgetController(NoteWidget *noteWidget):
            UMLWidgetController(noteWidget) {
    m_noteWidget = noteWidget;
}

NoteWidgetController::~NoteWidgetController() {
}

void NoteWidgetController::mouseMoveEvent(QMouseEvent *me) {
    UMLWidgetController::mouseMoveEvent(me);
    m_noteWidget->setEditorGeometry();
}

void NoteWidgetController::mouseReleaseEvent(QMouseEvent *me) {
    UMLWidgetController::mouseReleaseEvent(me);
    //TODO why is it needed? drawText is already called in draw,
    //and draw is (well, I think that is) called when the canvas rectangle is resized
    if (m_resized) {
        m_noteWidget->drawText();
    }
}

void NoteWidgetController::doMouseDoubleClick(QMouseEvent *me) {
    //TODO Copied from old code. What it does?
    if (m_noteWidget->m_DiagramLink == Uml::id_None) {
        m_noteWidget->slotMenuSelection(ListPopupMenu::mt_Rename);
    } else {
        UMLDoc *umldoc = UMLApp::app()->getDocument();
        umldoc->changeCurrentView(m_noteWidget->m_DiagramLink);
    }
}
