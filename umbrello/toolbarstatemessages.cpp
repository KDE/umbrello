/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "toolbarstatemessages.h"

#include <qevent.h>

#include "umlview.h"
#include "umldoc.h"

#include "objectwidget.h"
#include "floatingtext.h"
#include "messagewidget.h"
#include <qcanvas.h>

#include <kdebug.h>

#include <qwmatrix.h> // need for inverseWorldMatrix.map



ToolBarStateMessages::ToolBarStateMessages(UMLView *umlView) : ToolBarStatePool(umlView)	
{
	m_pSelectedWidget = NULL;
	m_pLine = NULL;
}

ToolBarStateMessages::~ToolBarStateMessages()
{
	removeLine();
}

void ToolBarStateMessages::removeLine()
{
	if( m_pLine ) 
	{
		delete m_pLine;
		m_pLine = NULL;
	}
}

void ToolBarStateMessages::mousePress(QMouseEvent* ome)
{
	ToolBarStatePool::mousePress(ome);

	removeLine();
}

/* Class holds the Sequence messages. */
void ToolBarStateMessages::mouseRelease(QMouseEvent* ome)
{
	m_pUMLView->m_pOnWidget = NULL;
	ToolBarStatePool::mouseRelease(ome);

	removeLine();

	ObjectWidget* clickedOnWidget = m_pUMLView->onWidgetLine( m_pMouseEvent->pos() );

	if (clickedOnWidget) 
	{
		if (!m_pSelectedWidget)
		{
			// First message
			m_pSelectedWidget = clickedOnWidget;

			m_pUMLView->viewport()->setMouseTracking( true );

			m_pLine = new QCanvasLine( m_pUMLView->canvas() );
			m_pLine->setPoints( m_pMouseEvent->x(), m_pMouseEvent->y(), m_pMouseEvent->x(), m_pMouseEvent->y() );
			m_pLine->setPen( QPen( m_pUMLView->getLineColor(), m_pUMLView->getLineWidth(), m_pUMLView->DashLine ) );
			m_pLine->setVisible( true );
		}
		else
		{
			//clicked on second sequence line to create message
			Uml::Text_Role tr = tr_Seq_Message;
			if (m_pSelectedWidget == clickedOnWidget)
				tr = tr_Seq_Message_Self;
			FloatingText* messageText = new FloatingText(m_pUMLView, tr, "");
			messageText->setFont( m_pUMLView->getFont() );

			ObjectWidget* pFirstSelectedObj = dynamic_cast<ObjectWidget*>(m_pSelectedWidget);
			if (pFirstSelectedObj == NULL) {
				kdDebug() << "first selected widget is not an object" << endl;
				return;
			}
			MessageWidget* message = new MessageWidget(m_pUMLView, pFirstSelectedObj,
								   clickedOnWidget, messageText,
								   m_pMouseEvent->y(),
								   getMessageType(),
								   m_pUMLView->getDocument()->getUniqueID());

			// TODO Do we really need a connect? It makes the code so hard to read. 
			m_pUMLView->connect(m_pUMLView, SIGNAL(sigColorChanged(int)), message, SLOT(slotColorChanged(int)));

			messageText->setLink( message );
			messageText->setActivated();
			message->setActivated();

			m_pSelectedWidget = 0;
			if (! m_pUMLView->m_WidgetList.contains(messageText))
				m_pUMLView->m_WidgetList.append(messageText);
			m_pUMLView->m_MessageList.append(message);
		}
	}
	else
	{
		m_pSelectedWidget = 0;
	}

	m_pUMLView->getDocument()->setModified();
}

// Override the ToolBarState::setSelectedWidget method.
bool ToolBarStateMessages::setSelectedWidget(QMouseEvent*) 
{
	return false;
}


	
void ToolBarStateMessages::mouseDoubleClick(QMouseEvent* ome)
{
	ToolBarStatePool::mouseDoubleClick(ome);
}

void ToolBarStateMessages::mouseMove(QMouseEvent* ome)
{
	ToolBarStatePool::mouseMove(ome);

	if( m_pLine ) 
	{
		QPoint sp = m_pLine -> startPoint();
		m_pLine -> setPoints( sp.x(), sp.y(), m_pMouseEvent->x(), m_pMouseEvent->y() );
	}
}

Sequence_Message_Type ToolBarStateMessages::getMessageType ()
{
	if (getButton() == WorkToolBar::tbb_Seq_Message_Synchronous)
		return sequence_message_synchronous; 

	return sequence_message_asynchronous;
} 


void ToolBarStateMessages::init()
{
	removeLine();
	m_pSelectedWidget = NULL;
	
	ToolBarStatePool::init();
}
