/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
