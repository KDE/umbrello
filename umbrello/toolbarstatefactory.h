/*

    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef TOOLBARSTATEFACTORY_H
#define TOOLBARSTATEFACTORY_H

#include "toolbarstate.h"

#include "worktoolbar.h"

#include <QPointer>
#define NR_OF_TOOLBAR_STATES 5



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

    ToolBarState* getState(const WorkToolBar::ToolBar_Buttons &toolbarButton, UMLScene* umlScene);

protected:
    int getKey(const WorkToolBar::ToolBar_Buttons &toolbarButton) const;

protected:
    QPointer<ToolBarState> m_states[NR_OF_TOOLBAR_STATES];
};

#endif //TOOLBARSTATEFACTORY_H
