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
#include "toolbarstatearrow.h"

#include <qevent.h>

#include "uml.h"
#include "umlview.h"
#include "umldoc.h"
#include <qptrlist.h>
#include <qwmatrix.h> // need for inverseWorldMatrix.map


ToolBarStateArrow::ToolBarStateArrow(UMLView *umlView): ToolBarState(umlView)
{
    m_SelectionRect.setAutoDelete( true );

    init();
}

ToolBarStateArrow::~ToolBarStateArrow()
{
    m_SelectionRect.clear();
}

void ToolBarStateArrow::mousePress(QMouseEvent* ome)
{
    ToolBarState::mousePress(ome);

    m_ButtonPressed = m_pMouseEvent->button();
    m_StartPosition = m_pMouseEvent->pos();

    // If we are not on a widget
    if (!m_bWidgetSelected && m_ButtonPressed == QMouseEvent::LeftButton)
    {
        m_bDrawRectangle = true;

        // Unselect all widgets.
        m_pUMLView->selectWidgets(0,0,0,0);

        // TODO  createSelectionRectangle
        for (int i = 0; i < 4; i++)
        {
            QCanvasLine* line = new QCanvasLine( m_pUMLView->canvas() );
            line->setPoints(m_pMouseEvent->x(), m_pMouseEvent->y(), m_pMouseEvent->x(), m_pMouseEvent->y());
            line->setPen( QPen(QColor("grey"), 0, m_pUMLView->DotLine) );
            line->setVisible(true);
            line->setZ(100);
            m_SelectionRect.append(line);
        }
    }
}

void ToolBarStateArrow::mouseRelease(QMouseEvent* ome)
{
    // The changeTool is reimplemented. (No functionality added)
    // Default behavior switches to tbb_arrow when the right button is pressed.

    ToolBarState::mouseRelease(ome);
}

void ToolBarStateArrow::changeTool()
{
    m_pUMLView->viewport()->setMouseTracking( false );

    if (m_pMouseEvent->state() == QMouseEvent::RightButton)
    {

        if (m_bWidgetSelected)
        {
            // TODO: This works. Actually, we want to show the menu only. And
            // not switch to the default tool
            UMLApp::app()->getWorkToolBar()->setDefaultTool();
        }
        else
        {
            m_pUMLView->setMenu();
        }
    }

    m_ButtonPressed = Qt::NoButton;

    m_SelectionRect.clear();
    m_bDrawRectangle = false;
}

void ToolBarStateArrow::mouseDoubleClick(QMouseEvent* ome)
{
    ToolBarState::mouseDoubleClick(ome);
}

void ToolBarStateArrow::mouseMove(QMouseEvent* ome)
{
    ToolBarState::mouseMove(ome);

    if (m_ButtonPressed == Qt::LeftButton && !m_pUMLView->onWidgetLine( m_pMouseEvent->pos()))
    {
        if( m_SelectionRect.count() == 4)
        {
            // TODO  updateSelectionRectangle
            QCanvasLine * line = m_SelectionRect.at( 0 );
            line -> setPoints( m_StartPosition.x(), m_StartPosition.y(), m_pMouseEvent->x(), m_StartPosition.y() );

            line = m_SelectionRect.at( 1 );
            line -> setPoints( m_pMouseEvent->x(), m_StartPosition.y(), m_pMouseEvent->x(), m_pMouseEvent->y() );

            line = m_SelectionRect.at( 2 );
            line -> setPoints( m_pMouseEvent->x(), m_pMouseEvent->y(), m_StartPosition.x(), m_pMouseEvent->y() );

            line = m_SelectionRect.at( 3 );
            line -> setPoints( m_StartPosition.x(), m_pMouseEvent->y(), m_StartPosition.x(), m_StartPosition.y() );

            m_pUMLView->selectWidgets(m_StartPosition.x(),m_StartPosition.y(), m_pMouseEvent->x(), m_pMouseEvent->y());
        }
    }
}

void ToolBarStateArrow::init()
{
    ToolBarState::init();

    m_ButtonPressed = Qt::NoButton;
    m_bDrawRectangle = false;
}

