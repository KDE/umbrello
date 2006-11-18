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

#ifndef TOOLBARSTATEPOOL_H
#define TOOLBARSTATEPOOL_H

#include "toolbarstate.h"

#include "worktoolbar.h"

/**
 * Base class for tools that can use the same state but with different button.
 * This class only adds support to specify the button currently in use for a
 * tool bar state.
 */
class ToolBarStatePool : public ToolBarState {
    Q_OBJECT
public:

    /**
     * Destroys this ToolBarStatePool.
     */
    virtual ~ToolBarStatePool();

    /**
     * Sets the current button and inits the tool.
     * If the current button is the same to the button to set, the tool isn't
     * initialized.
     *
     * @param button The button to set.
     */
    void setButton(const WorkToolBar::ToolBar_Buttons &button);

    /**
     * Returns the current button.
     *
     * @return The current button.
     */
    WorkToolBar::ToolBar_Buttons getButton() const {
        return m_ToolBarButton;
    }

protected:

    /**
     * Creates a new ToolBarStatePool.
     * Protected to avoid classes other than derived to create objects of this
     * class.
     *
     * @param umlView The UMLView to use.
     */
    ToolBarStatePool(UMLView *umlView);

private:

    /**
     * The current button of the tool.
     */
    WorkToolBar::ToolBar_Buttons m_ToolBarButton;

};

#endif //TOOLBARSTATEPOOL_H
