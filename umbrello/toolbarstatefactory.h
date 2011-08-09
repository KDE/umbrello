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

#ifndef TOOLBARSTATEFACTORY_H
#define TOOLBARSTATEFACTORY_H

#include "toolbarstate.h"

#include "worktoolbar.h"

#define NR_OF_TOOLBAR_STATES 5

class UMLView;

//new canvas
#define SOC2011 1
namespace QGV {
  class UMLView;
}

/**
 * The ToolBarStateFactory keeps track of all the toolbar states. For the first
 * request, the factory creates a new state object. The next requests to this
 * object, this factory will return the existing object.
 *
 * States that inherit from the ToolBarStatePool share multiple toolbar states.
 * Therefore the setButton function is called. Internally the shared state object
 * determines the exact behavior by itself.
 */
class ToolBarStateFactory
{
public:
    // constructor.
    ToolBarStateFactory();

    // Destructor
    virtual ~ToolBarStateFactory();

    ToolBarState* getState(const WorkToolBar::ToolBar_Buttons &toolbarButton, UMLView* umlView);
    
#ifdef SOC2011
    ToolBarState* state(const WorkToolBar::ToolBar_Buttons &toolbarButton, QGV::UMLView* umlView);
#endif

protected:
    int getKey(const WorkToolBar::ToolBar_Buttons &toolbarButton) const;

protected:
    ToolBarState* m_states[NR_OF_TOOLBAR_STATES];
};

#endif //TOOLBARSTATEFACTORY_H
