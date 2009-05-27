/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2009                                               *
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
class ToolBarStatePool : public ToolBarState
{
    Q_OBJECT
public:

    virtual ~ToolBarStatePool();

    void setButton(const WorkToolBar::ToolBar_Buttons &button);
    WorkToolBar::ToolBar_Buttons getButton() const;

protected:

    ToolBarStatePool(UMLScene *umlScene);

private:

    WorkToolBar::ToolBar_Buttons m_ToolBarButton;  ///< The current button of the tool.

};

#endif //TOOLBARSTATEPOOL_H
