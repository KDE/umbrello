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
#include <qmatrix.h> // need for inverseWorldMatrix.map
#include <qevent.h>

#include <kdebug.h>

#include "toolbarstate.h"
#include "umlview.h"
#include "umlwidget.h"
#include "messagewidget.h"
#include "associationwidget.h"
#include "uml.h"

ToolBarState::ToolBarState(UMLView *umlView) : m_pUMLView(umlView)
{
    m_pMouseEvent = NULL;
    m_bWidgetSelected = false;
    init();
}

ToolBarState::~ToolBarState()
{
    if (m_pMouseEvent) delete m_pMouseEvent;
}

void ToolBarState::init()
{
    m_pUMLView->viewport()->setMouseTracking( false );
    m_bIsButtonPressed = false;
}

void ToolBarState::setMouseEvent (QMouseEvent* ome, const QEvent::Type &type)
{
    if (m_pMouseEvent) delete m_pMouseEvent;

    m_pMouseEvent = new QMouseEvent(type, m_pUMLView->inverseWorldMatrix().map(ome->pos()),
                                    ome->button(),ome->buttons(),ome->modifiers());
}

void ToolBarState::mousePress(QMouseEvent* ome)
{
    setMouseEvent (ome, QEvent::MouseButtonPress);
    m_bIsButtonPressed = true;

    m_pUMLView->viewport()->setMouseTracking(true);

    // TODO: emit sucks
    m_pUMLView->emitRemovePopupMenu();

    // TODO: Check who needs this.
    m_pUMLView->setPos( m_pMouseEvent->pos() );

    m_pUMLView->setPaste(false);

    setSelectedWidget(m_pMouseEvent);
}

void ToolBarState::mouseRelease(QMouseEvent* ome)
{
    setMouseEvent (ome, QEvent::MouseButtonRelease);
    m_bIsButtonPressed = false;

    // Set the position of the mouse
    // TODO, should only be available in this state?
    m_pUMLView->getPos().setX(m_pMouseEvent->x());
    m_pUMLView->getPos().setY(m_pMouseEvent->y());

    // TODO: Should not be called by an Sequence message Association.
    UMLWidget *onW = m_pUMLView->getOnWidget();
    if (onW)
        onW->mouseReleaseEvent(m_pMouseEvent);
    AssociationWidget *moveAssoc = m_pUMLView->getMoveAssoc();
    if (moveAssoc)
        moveAssoc->mouseReleaseEvent(m_pMouseEvent);

    // Default, rightbutton changes the tool.
    // The arrow tool overrides the changeTool() function.
    changeTool();
}

void ToolBarState::changeTool()
{
    if (m_pMouseEvent->button() == Qt::RightButton)
    {
        /* if the user right clicks on the diagram, first the default tool is
         * selected from the toolbar; this only happens when the default tool
         * wasn't selected yet AND there is no other widget under the mouse
         * pointer
         * in any other case the right click menu will be shown
         * */
        UMLApp::app()->getWorkToolBar()->setDefaultTool();
    }

    if (m_pMouseEvent->button() != Qt::LeftButton)
    {
        m_pUMLView->viewport()->setMouseTracking( false );
    }
}


void ToolBarState::mouseDoubleClick(QMouseEvent* ome)
{
    setMouseEvent(ome, QEvent::MouseButtonDblClick);


    UMLWidget *onW = m_pUMLView->getOnWidget();
    AssociationWidget *moveAssoc = m_pUMLView->getMoveAssoc();
    if (onW && onW->onWidget( m_pMouseEvent->pos()))
    {
        onW->mouseDoubleClickEvent( m_pMouseEvent );
    }
    else if (moveAssoc && moveAssoc->onAssociation( m_pMouseEvent->pos() ))
    {
        moveAssoc->mouseDoubleClickEvent( m_pMouseEvent );
    }
    else
    {
        m_pUMLView->setOnWidget(NULL);
        m_pUMLView->setMoveAssoc(NULL);
        m_pUMLView->clearSelected();
    }
}

void ToolBarState::mouseMove(QMouseEvent* ome)
{
    setMouseEvent(ome, QEvent::MouseMove);

    UMLWidget *onW = m_pUMLView->getOnWidget();
    AssociationWidget *moveAssoc = m_pUMLView->getMoveAssoc();
    if (onW)
        onW->mouseMoveEvent( m_pMouseEvent );
    else if (moveAssoc)
        moveAssoc->mouseMoveEvent( m_pMouseEvent );

    if (m_bIsButtonPressed)
    {
        int vx = ome->x();
        int vy = ome->y();
        int contsX = m_pUMLView->contentsX();
        int contsY = m_pUMLView->contentsY();
        int visw = m_pUMLView->visibleWidth();
        int vish = m_pUMLView->visibleHeight();
        int dtr = visw - (vx-contsX);
        int dtb = vish - (vy-contsY);
        int dtt =  (vy-contsY);
        int dtl =  (vx-contsX);
        if (dtr < 30) m_pUMLView->scrollBy(30-dtr,0);
        if (dtb < 30) m_pUMLView->scrollBy(0,30-dtb);
        if (dtl < 30) m_pUMLView->scrollBy(-(30-dtl),0);
        if (dtt < 30) m_pUMLView->scrollBy(0,-(30-dtt));
    }

}

// TODO: Remove parameter?
bool ToolBarState::setSelectedWidget(QMouseEvent * me)
{
    m_pUMLView->setMoveAssoc(NULL);
    m_pUMLView->setOnWidget(NULL);

    // Check associations.
    AssociationWidgetListIt assoc_it(m_pUMLView->getAssociationList());
    AssociationWidget* assocwidget = 0;
    while ((assocwidget = assoc_it.current()) != NULL) {
        if (assocwidget->onAssociation( me->pos() ))
        {
            // TODO: Fix this. It makes a callback to the association mousePressEvent function.
            assocwidget->mousePressEvent(me);
            m_pUMLView->setMoveAssoc(assocwidget);
            m_bWidgetSelected = true;
            return true;
        }
        ++assoc_it;
    }
    m_pUMLView->setMoveAssoc(NULL);

    // Check messages.
    for (MessageWidgetListIt mit(m_pUMLView->getMessageList()); mit.current(); ++mit) {
        MessageWidget *obj = mit.current();
        if (obj->isVisible() && obj->onWidget(me->pos())) {
            m_pUMLView->setOnWidget( obj );
            obj->mousePressEvent( me );
            m_bWidgetSelected = true;
            return true;
        }
    }

    // Check widgets.
    UMLWidget *smallestObj = 0;
    int relativeSize = 10000;   // start with an arbitrary large number
    for (UMLWidgetListIt it(m_pUMLView->getWidgetList()); it.current(); ++it) {
        UMLWidget *obj = it.current();
        if (!obj->isVisible())
            continue;
        const int s = obj->onWidget(me->pos());
        if (!s)
            continue;
        if (s < relativeSize) {
            relativeSize = s;
            smallestObj = obj;
        }
    }
    if (smallestObj) {
        m_pUMLView->setOnWidget(smallestObj);
        smallestObj->mousePressEvent(me);
        m_bWidgetSelected = true;
        return true;
    }

    m_pUMLView->setOnWidget(NULL);

    m_bWidgetSelected = false;
    return false;
}
