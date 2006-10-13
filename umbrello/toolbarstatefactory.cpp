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
#include "toolbarstatefactory.h"

#include "toolbarstate.h"
#include "toolbarstatepool.h"
#include "toolbarstateother.h"
#include "toolbarstatearrow.h"
#include "toolbarstatemessages.h"
#include "toolbarstateassociation.h"

#include "umlview.h"

ToolBarStateFactory::ToolBarStateFactory(UMLView *umlView)
{
    m_pUMLView = umlView;

    for (int i = 0; i < NR_OF_TOOLBAR_STATES; i++)
    {
        states[i] = NULL;
    }
}

ToolBarStateFactory::~ToolBarStateFactory()
{
    for (int i = 0; i < NR_OF_TOOLBAR_STATES; i++)
    {
        if (states[i] != NULL) delete states[i];
    }
}


ToolBarState* ToolBarStateFactory::getState(const WorkToolBar::ToolBar_Buttons &toolbarButton)
{
    int key = getKey(toolbarButton);

    if (states[key] == NULL)
    {
        switch (key)
        {
            // When you add a new state, make sure you also increase the
            // NR_OF_TOOLBAR_STATES
        case 0: states[0] = new ToolBarStateOther(m_pUMLView); break;
        case 1: states[1] = new ToolBarStateAssociation(m_pUMLView); break;
        case 2: states[2] = new ToolBarStateMessages(m_pUMLView); break;

            // This case has no pool.
        case 3: states[3] = new ToolBarStateArrow(m_pUMLView); break;
        }
    }

    // Make explicit the selected button. This is only necessary for states with a pool.
    if (key <= 2) ((ToolBarStatePool *) states[key])->setButton(toolbarButton);

    return states[key];
}


int ToolBarStateFactory::getKey(const WorkToolBar::ToolBar_Buttons &toolbarButton) const
{
    switch (toolbarButton)
    {
        // Associations
    case WorkToolBar::tbb_Dependency:          return 1;
    case WorkToolBar::tbb_Aggregation:         return 1;
    case WorkToolBar::tbb_Relationship:        return 1;
    case WorkToolBar::tbb_Generalization:      return 1;
    case WorkToolBar::tbb_Association:         return 1;
    case WorkToolBar::tbb_UniAssociation:      return 1;
    case WorkToolBar::tbb_Composition:         return 1;
    case WorkToolBar::tbb_Containment:         return 1;
    case WorkToolBar::tbb_Anchor:              return 1;
    case WorkToolBar::tbb_Coll_Message:        return 1;
    case WorkToolBar::tbb_State_Transition:    return 1;
    case WorkToolBar::tbb_Activity_Transition: return 1;

        // Messages
    case WorkToolBar::tbb_Seq_Message_Synchronous:  return 2;
    case WorkToolBar::tbb_Seq_Message_Asynchronous: return 2;

        // Arrow pointer
    case WorkToolBar::tbb_Arrow: return 3;

        // Other.
    default: return 0;
    }

}
