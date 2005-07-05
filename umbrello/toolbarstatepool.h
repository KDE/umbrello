/*
 *  copyright (C) 2004
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TOOLBARSTATEPOOL_H
#define TOOLBARSTATEPOOL_H

#include "toolbarstate.h"

#include "worktoolbar.h"

class QMouseEvent;
class UMLView;
class UMLWidget;

class ToolBarStatePool : public ToolBarState
{

public:
    ToolBarStatePool(UMLView *umlView);

    virtual void setButton(const WorkToolBar::ToolBar_Buttons &button);
    virtual WorkToolBar::ToolBar_Buttons getButton() const;

private:
    WorkToolBar::ToolBar_Buttons m_ToolBarButton;
};

#endif //TOOLBARSTATEPOOL_H
