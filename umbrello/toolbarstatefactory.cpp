/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/
#include "toolbarstatefactory.h"

#include "toolbarstate.h"
#include "toolbarstatepool.h"
#include "toolbarstateother.h"
#include "toolbarstatearrow.h"
#include "toolbarstatemessages.h"
#include "toolbarstateassociation.h"
#include "toolbarstateonewidget.h"

#include "umlview.h"

ToolBarStateFactory::ToolBarStateFactory()
{
    for (int i = 0; i < NR_OF_TOOLBAR_STATES; ++i)
    {
        m_states[i] = 0;
    }
}

ToolBarStateFactory::~ToolBarStateFactory()
{
    for (int i = 0; i < NR_OF_TOOLBAR_STATES; ++i)
    {
        if (m_states[i])
            delete m_states[i];
    }
}


ToolBarState* ToolBarStateFactory::getState(const WorkToolBar::ToolBar_Buttons &toolbarButton, UMLScene *umlScene)
{
    int key = getKey(toolbarButton);

    if (m_states[key] == 0)
    {
        switch (key)
        {
            // When you add a new state, make sure you also increase the
            // NR_OF_TOOLBAR_STATES
        case 0: m_states[0] = new ToolBarStateOther(umlScene); break;
        case 1: m_states[1] = new ToolBarStateAssociation(umlScene); break;
        case 2: m_states[2] = new ToolBarStateMessages(umlScene); break;

            // This case has no pool.
        case 3: m_states[3] = new ToolBarStateArrow(umlScene); break;
        case 4: m_states[4] = new ToolBarStateOneWidget(umlScene); break;
        }
    }

    // Make explicit the selected button. This is only necessary for states with a pool.
    if (key != 3) ((ToolBarStatePool *) m_states[key])->setButton(toolbarButton);

    return m_states[key];
}


int ToolBarStateFactory::getKey(const WorkToolBar::ToolBar_Buttons &toolbarButton) const
{
    switch (toolbarButton)
    {
        // Associations
    case WorkToolBar::tbb_Dependency:               return 1;
    case WorkToolBar::tbb_Aggregation:              return 1;
    case WorkToolBar::tbb_Relationship:             return 1;
    case WorkToolBar::tbb_Generalization:           return 1;
    case WorkToolBar::tbb_Association:              return 1;
    case WorkToolBar::tbb_UniAssociation:           return 1;
    case WorkToolBar::tbb_Composition:              return 1;
    case WorkToolBar::tbb_Containment:              return 1;
    case WorkToolBar::tbb_Anchor:                   return 1;
    case WorkToolBar::tbb_Coll_Message:             return 1;
    case WorkToolBar::tbb_State_Transition:         return 1;
    case WorkToolBar::tbb_Activity_Transition:      return 1;
    case WorkToolBar::tbb_Exception:                return 1;
    case WorkToolBar::tbb_Category2Parent:          return 1;
    case WorkToolBar::tbb_Child2Category:           return 1;

        // Messages
    case WorkToolBar::tbb_Seq_Message_Synchronous:  return 2;
    case WorkToolBar::tbb_Seq_Message_Asynchronous: return 2;
    case WorkToolBar::tbb_Seq_Message_Found:        return 2;
    case WorkToolBar::tbb_Seq_Message_Lost:         return 2;

    case WorkToolBar::tbb_Seq_Precondition:         return 4;
    case WorkToolBar::tbb_Pin:                      return 4;

        // Arrow pointer
    case WorkToolBar::tbb_Arrow:                    return 3;

        // Other.
    default:                                        return 0;
    }

}
