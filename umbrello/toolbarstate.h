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

#ifndef TOOLBARSTATE_H
#define TOOLBARSTATE_H

#include <qevent.h>

class QMouseEvent;
class UMLView;
class UMLWidget;
class AssociationWidget;
class QEvent;


/**
 * All toolbar states inherit directly or indirectly from this class. 
 */
class ToolBarState
{

public:
    ToolBarState(UMLView *umlView);
    virtual ~ToolBarState();

    virtual void mousePress(QMouseEvent *ome);
    virtual void mouseRelease(QMouseEvent*);
    virtual void mouseDoubleClick(QMouseEvent*);
    virtual void mouseMove(QMouseEvent*);

    // Go back to the initial state.
    virtual void init();

protected:

    virtual void setMouseEvent (QMouseEvent* ome, const QEvent::Type &type);
    virtual bool setSelectedWidget(QMouseEvent * me);
    virtual void changeTool();

    UMLView* m_pUMLView;

    QMouseEvent* m_pMouseEvent;
    bool m_bWidgetSelected;

    bool m_bIsButtonPressed;
};

#endif //TOOLBARSTATE_H
