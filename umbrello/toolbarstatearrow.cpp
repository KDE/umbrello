/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "toolbarstatearrow.h"

// app includes
#include "umlscene.h"

/**
 * Creates a new ToolBarStateArrow.
 *
 * @param umlScene The UMLScene to use.
 */
ToolBarStateArrow::ToolBarStateArrow(UMLScene *umlScene)
  : ToolBarState(umlScene)
{
}

void ToolBarStateArrow::mousePress(QGraphicsSceneMouseEvent *ome)
{
    m_pUMLScene->callBaseMouseMethod(ome);
}

void ToolBarStateArrow::mouseMove(QGraphicsSceneMouseEvent *ome)
{
    m_pUMLScene->callBaseMouseMethod(ome);
}

void ToolBarStateArrow::mouseRelease(QGraphicsSceneMouseEvent *ome)
{
    m_pUMLScene->callBaseMouseMethod(ome);
}

void ToolBarStateArrow::mouseDoubleClick(QGraphicsSceneMouseEvent *ome)
{
    m_pUMLScene->callBaseMouseMethod(ome);
}

#include "toolbarstatearrow.moc"
