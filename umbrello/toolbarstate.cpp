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
#include <qwmatrix.h> // need for inverseWorldMatrix.map

#include "toolbarstate.h"
#include "umlview.h"
#include "umlwidget.h"
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
					  ome->button(),ome->state());
}

void ToolBarState::mousePress(QMouseEvent* ome)
{
	setMouseEvent (ome, QEvent::MouseButtonPress);
	m_bIsButtonPressed = true;

	m_pUMLView->viewport()->setMouseTracking(true);

	// TODO: emit sucks
	emit m_pUMLView->sigRemovePopupMenu();

	// TODO: Check who needs this.
	m_pUMLView->m_Pos = m_pMouseEvent->pos();

	if(m_pUMLView->m_bPaste) m_pUMLView->m_bPaste = false;

	setSelectedWidget(m_pMouseEvent);
}

void ToolBarState::mouseRelease(QMouseEvent* ome)
{
	setMouseEvent (ome, QEvent::MouseButtonRelease);
	m_bIsButtonPressed = false;

	// Set the position of the mouse
	// TODO, should only be available in this state?
	m_pUMLView->m_Pos.setX(m_pMouseEvent->x());
	m_pUMLView->m_Pos.setY(m_pMouseEvent->y());

	// TODO: Should not be called by an Sequence message Association.
	if(m_pUMLView->m_pOnWidget) m_pUMLView->m_pOnWidget->mouseReleaseEvent(m_pMouseEvent);
	if(m_pUMLView->m_pMoveAssoc) m_pUMLView->m_pMoveAssoc->mouseReleaseEvent(m_pMouseEvent);

	// Default, rightbutton changes the tool.
	// The arrow tool overrides the changeTool() function.
	changeTool();
}

void ToolBarState::changeTool()
{
	if (m_pMouseEvent->state() == QMouseEvent::RightButton)
	{
		/* if the user right clicks on the diagram, first the default tool is
		 * selected from the toolbar; this only happens when the default tool
		 * wasn't selected yet AND there is no other widget under the mouse
		 * pointer
		 * in any other case the right click menu will be shown
		 * */
		UMLApp::app()->getWorkToolBar()->setDefaultTool();
	}

	if (m_pMouseEvent->state() != QMouseEvent::LeftButton)
	{
		m_pUMLView->viewport()->setMouseTracking( false );
	}
}


void ToolBarState::mouseDoubleClick(QMouseEvent* ome)
{
	setMouseEvent(ome, QEvent::MouseButtonDblClick);


	if( m_pUMLView->m_pOnWidget && m_pUMLView->m_pOnWidget->onWidget( m_pMouseEvent->pos()))
	{
		m_pUMLView->m_pOnWidget->mouseDoubleClickEvent( m_pMouseEvent );
	}
	else if( m_pUMLView->m_pMoveAssoc && m_pUMLView->m_pMoveAssoc->onAssociation( m_pMouseEvent -> pos() ))
	{
		m_pUMLView->m_pMoveAssoc->mouseDoubleClickEvent( m_pMouseEvent );
	}
	else
	{
		m_pUMLView->m_pOnWidget = 0;
		m_pUMLView->m_pMoveAssoc = 0;
		m_pUMLView->clearSelected();
	}
}

void ToolBarState::mouseMove(QMouseEvent* ome)
{
	setMouseEvent(ome, QEvent::MouseMove);

	if (m_pUMLView->m_pOnWidget)
		m_pUMLView->m_pOnWidget->mouseMoveEvent( m_pMouseEvent );
	else if (m_pUMLView->m_pMoveAssoc)
		m_pUMLView->m_pMoveAssoc->mouseMoveEvent( m_pMouseEvent );

	if (m_bIsButtonPressed)
	{
		int vx = m_pMouseEvent->x();
		int vy = m_pMouseEvent->y();
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
	m_pUMLView->m_pMoveAssoc = 0;
	m_pUMLView->m_pOnWidget = 0;

	UMLWidget* backup = 0;
	UMLWidget* boxBackup = 0;

	// Check widgets.
	UMLWidgetListIt it( m_pUMLView->m_WidgetList );
	UMLWidget* obj = 0;
	while ( (obj = it.current()) != 0 ) {
		++it;
		if( !obj->isVisible() || !obj->onWidget(me->pos()) )
			continue;
		//Give text object priority,
		//they can easily get into a position where
		//you can't touch them.
		//Give Boxes lowest priority, we want to be able to move things that
		//are on top of them.
		if (obj->getBaseType() == Uml::wt_Text)
		{
			m_pUMLView->m_pOnWidget = obj;
			obj ->  mousePressEvent( me );
			m_bWidgetSelected = true;
			return true;
		} else if (obj->getBaseType() == Uml::wt_Box) {
			boxBackup = obj;
		} else {
			backup = obj;
		}
	}//end while
	//if backup is set then let it have the event
	if(backup) {
		backup -> mousePressEvent( me );
		m_pUMLView->m_pOnWidget = backup;

		m_bWidgetSelected = true;
		return true;
	}

	// Check messages.
	MessageWidgetListIt mit( m_pUMLView->m_MessageList );
	obj = 0;
	while ((obj = (UMLWidget*)mit.current()) != 0) {
		if (obj->isVisible() && obj->onWidget(me->pos())) {
			m_pUMLView->m_pOnWidget = obj;
			obj ->  mousePressEvent( me );
			m_bWidgetSelected = true;
			return true;
		}
		++mit;
	}

	// Boxes have lower priority.
	if (boxBackup) {
		boxBackup -> mousePressEvent( me );
		m_pUMLView->m_pOnWidget = boxBackup;

		m_bWidgetSelected = true;
		return true;
	}

	// Check associations.
	AssociationWidgetListIt assoc_it(m_pUMLView->m_AssociationList);
	AssociationWidget* assocwidget = 0;
	while((assocwidget=assoc_it.current())) {
		if( assocwidget -> onAssociation( me -> pos() ))
		{
			// TODO: Fix this. It makes a callback to the association mousePressEvent function.
			assocwidget->mousePressEvent(me);
			m_pUMLView->m_pMoveAssoc = assocwidget;

			m_bWidgetSelected = true;
			return true;
		}
		++assoc_it;
	}
	m_pUMLView->m_pMoveAssoc = 0;
	m_pUMLView->m_pOnWidget = 0;

	m_bWidgetSelected = false;
	return false;
}
