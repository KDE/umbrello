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
// own header
#include "toolbarstatemessages.h"

// system includes
#include <qevent.h>
#include <q3canvas.h>
#include <qmatrix.h> // need for inverseWorldMatrix.map
//Added by qt3to4:
#include <QMouseEvent>
#include <kdebug.h>

// local includes
#include "umlview.h"
#include "umldoc.h"
#include "uml.h"
#include "objectwidget.h"
#include "floatingtext.h"
#include "messagewidget.h"


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
    m_pUMLView->setOnWidget(NULL);
    ToolBarStatePool::mouseRelease(ome);

    removeLine();

    ObjectWidget* clickedOnWidget = m_pUMLView->onWidgetLine( m_pMouseEvent->pos() );
    bool isCreationMessage = false;

    if (clickedOnWidget == NULL && m_pSelectedWidget) {
        // Check ObjectWidgets - for creation message.
        UMLWidgetListIt it( m_pUMLView->getWidgetList() );
        UMLWidget* obj = 0;
        while ( (obj = it.current()) != 0 ) {
            ++it;
            if ( obj->isVisible() &&
                    obj->getBaseType() == Uml::wt_Object &&
                    obj->onWidget(ome->pos()) ) {
                clickedOnWidget = static_cast<ObjectWidget*>(obj);
                isCreationMessage = true;
                break;
            }
        }
    }
    if (clickedOnWidget)
    {
        if (!m_pSelectedWidget)
        {
            // First object
            m_pSelectedWidget = clickedOnWidget;
            m_FirstMousePos = m_pMouseEvent->pos();

            m_pUMLView->viewport()->setMouseTracking( true );

            m_pLine = new Q3CanvasLine( m_pUMLView->canvas() );
            m_pLine->setPoints( m_pMouseEvent->x(), m_pMouseEvent->y(), m_pMouseEvent->x(), m_pMouseEvent->y() );
            m_pLine->setPen( QPen( m_pUMLView->getLineColor(), m_pUMLView->getLineWidth(), m_pUMLView->DashLine ) );
            m_pLine->setVisible( true );
        }
        else
        {
            // Second object
            ObjectWidget* pFirstSelectedObj = dynamic_cast<ObjectWidget*>(m_pSelectedWidget);
            if (pFirstSelectedObj == NULL) {
                kdDebug() << "first selected widget is not an object" << endl;
                return;
            }
            Uml::Sequence_Message_Type msgType = getMessageType();
            int y = m_pMouseEvent->y();
            if (isCreationMessage) {
                msgType = Uml::sequence_message_creation;
                y = m_FirstMousePos.y();
            }
            MessageWidget* message = new MessageWidget(m_pUMLView, pFirstSelectedObj,
                                     clickedOnWidget, y, msgType);

            // TODO Do we really need a connect? It makes the code so hard to read.
            m_pUMLView->connect(m_pUMLView, SIGNAL(sigColorChanged(Uml::IDType)),
                                message, SLOT(slotColorChanged(Uml::IDType)));

            message->activate();

            m_pSelectedWidget = 0;
            m_pUMLView->getMessageList().append(message);
            FloatingText *ft = message->getFloatingText();
            ft->showOpDlg();
            message->setTextPosition();
            m_pUMLView->getWidgetList().append(ft);
        }
    }
    else
    {
        m_pSelectedWidget = 0;
    }

    UMLApp::app()->getDocument()->setModified();
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

Uml::Sequence_Message_Type ToolBarStateMessages::getMessageType ()
{
    if (getButton() == WorkToolBar::tbb_Seq_Message_Synchronous)
        return Uml::sequence_message_synchronous;

    return Uml::sequence_message_asynchronous;
}


void ToolBarStateMessages::init()
{
    removeLine();
    m_pSelectedWidget = NULL;

    ToolBarStatePool::init();
}
