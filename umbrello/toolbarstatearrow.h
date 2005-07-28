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

#ifndef TOOLBARSTATEARROW_H
#define TOOLBARSTATEARROW_H


#include "toolbarstate.h"

#include "worktoolbar.h"
//Added by qt3to4:
#include <QMouseEvent>
#include <Q3PtrList>

class QMouseEvent;
class UMLView;

class Q3CanvasLine;

class ToolBarStateArrow : public ToolBarState
{

public:
    ToolBarStateArrow(UMLView *umlView);
    virtual ~ToolBarStateArrow();

    virtual void mousePress(QMouseEvent* ome);
    virtual void mouseRelease(QMouseEvent* ome);
    virtual void mouseDoubleClick(QMouseEvent* ome);
    virtual void mouseMove(QMouseEvent* ome);

    virtual void init();
protected:

    virtual void changeTool();

    Q3PtrList<Q3CanvasLine> m_SelectionRect;
    bool m_bDrawRectangle;

    Qt::ButtonState m_ButtonPressed;
    QPoint m_StartPosition;
};

#endif //TOOLBARSTATEARROW_H
