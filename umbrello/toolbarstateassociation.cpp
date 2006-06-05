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
#include "toolbarstateassociation.h"

#include <qevent.h>
//Added by qt3to4:
#include <QMouseEvent>

#include "umlview.h"
#include "umldoc.h"

#include "objectwidget.h"
#include "floatingtextwidget.h"
#include "messagewidget.h"
#include <q3canvas.h>

#include <kdebug.h>

#include <qmatrix.h> // needed for inverseWorldMatrix.map


ToolBarStateAssociation::ToolBarStateAssociation(UMLView *umlView) : ToolBarStatePool(umlView)
{
}

ToolBarStateAssociation::~ToolBarStateAssociation()
{
}

void ToolBarStateAssociation::mousePress(QMouseEvent* ome)
{
    ToolBarStatePool::mousePress(ome);

    Q3CanvasLine *line = m_pUMLView->getAssocLine();
    if (line)
    {
        delete line;
        m_pUMLView->setAssocLine(NULL);
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

    if( m_pUMLView->getFirstSelectedWidget() )
    {
        Q3CanvasLine *line = m_pUMLView->getAssocLine();
        if (line)
        {
            QPoint sp = line->startPoint();
            line->setPoints( sp.x(), sp.y(), m_pMouseEvent->x(), m_pMouseEvent->y() );
        }
    }
}

void ToolBarStateAssociation::init()
{
    m_pUMLView->setFirstSelectedWidget( NULL );

    Q3CanvasLine *line = m_pUMLView->getAssocLine();
    if (line)
    {
        delete line;
        m_pUMLView->setAssocLine( NULL );
    }

    ToolBarStatePool::init();
}

