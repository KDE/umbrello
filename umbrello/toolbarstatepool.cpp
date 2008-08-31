/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "toolbarstatepool.h"

ToolBarStatePool::~ToolBarStatePool() {
}

/**
 * Sets the current button and inits the tool.
 * If the current button is the same to the button to set, the tool isn't
 * initialized.
 *
 * @param button The button to set.
 */
void ToolBarStatePool::setButton(const WorkToolBar::ToolBar_Buttons &button) {
    if (button != m_ToolBarButton) {
        m_ToolBarButton = button;

        init(); // Go back to the initial state.
    }
}

ToolBarStatePool::ToolBarStatePool(UMLScene *umlScene): ToolBarState(umlScene)
{
    m_ToolBarButton = WorkToolBar::tbb_Arrow;
}

#include "toolbarstatepool.moc"
