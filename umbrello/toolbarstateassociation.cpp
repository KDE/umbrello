/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "toolbarstateassociation.h"

#include <qevent.h>

#include "umlview.h"
#include "umldoc.h"

#include "objectwidget.h"
#include "floatingtext.h"
#include "messagewidget.h"
#include <qcanvas.h>

#include <kdebug.h>

#include <qwmatrix.h> // needed for inverseWorldMatrix.map


ToolBarStateAssociation::ToolBarStateAssociation(UMLView *umlView) : ToolBarStatePool(umlView)	
{
}

ToolBarStateAssociation::~ToolBarStateAssociation()
{
}

void ToolBarStateAssociation::mousePress(QMouseEvent* ome)
{
	ToolBarStatePool::mousePress(ome);

	if( m_pUMLView->m_pAssocLine) 
	{
		delete m_pUMLView->m_pAssocLine;
		m_pUMLView->m_pAssocLine = NULL;
	}
}

void ToolBarStateAssociation::mouseRelease(QMouseEvent* ome)
{
	ToolBarStatePool::mouseRelease(ome);
}

	
void ToolBarStateAssociation::mouseDoubleClick(QMouseEvent* ome)
{
	ToolBarStatePool::mouseDoubleClick(ome);
}

void ToolBarStateAssociation::mouseMove(QMouseEvent* ome)
{
	ToolBarStatePool::mouseMove(ome);

	// TODO: Should be encapsulated.
	if( m_pUMLView->m_pFirstSelectedWidget ) 
	{
		if( m_pUMLView->m_pAssocLine ) 
		{
			QPoint sp = m_pUMLView->m_pAssocLine -> startPoint();
			m_pUMLView->m_pAssocLine -> setPoints( sp.x(), sp.y(), m_pMouseEvent->x(), m_pMouseEvent->y() );
		}
	}
}

void ToolBarStateAssociation::init()
{
	m_pUMLView->m_pFirstSelectedWidget = NULL;
		
	if (m_pUMLView->m_pAssocLine) 
	{
		delete m_pUMLView->m_pAssocLine;
		m_pUMLView->m_pAssocLine = NULL;
	}

	ToolBarStatePool::init();
}
