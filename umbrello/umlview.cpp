/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umlview.h"
#include "umlscene.h"
#include "uml.h"

/**
 * Constructor
 */
UMLView::UMLView(UMLFolder *parentFolder)
  : Q3CanvasView(UMLApp::app()->mainViewWidget()),
    m_scene(new UMLScene(parentFolder, this)),
    m_nZoom(100)
{
    setCanvas(m_scene);
}

/**
 * Destructor
 */
UMLView::~UMLView()
{
}

/**
 * Sets the zoom of the diagram.
 */
void UMLView::setZoom(int zoom)
{
    if (zoom < 10) {
        zoom = 10;
    } else if (zoom > 500) {
        zoom = 500;
    }
    
    QMatrix wm;
    wm.scale(zoom / 100.0, zoom / 100.0);
    setWorldMatrix(wm);
    
    m_nZoom = currentZoom();
    m_scene->resizeCanvasToItems();
}

/**
 * return the current zoom factor
 */
int UMLView::currentZoom()
{
    return (int)(worldMatrix().m11()*100.0);
}

void UMLView::zoomIn()
{
    QMatrix wm = worldMatrix();
    wm.scale(1.5, 1.5); // adjust zooming step here
    setZoom((int)(wm.m11()*100.0));
}

void UMLView::zoomOut()
{
    QMatrix wm = worldMatrix();
    wm.scale(2.0 / 3.0, 2.0 / 3.0); //adjust zooming step here
    setZoom((int)(wm.m11()*100.0));
}

/**
 * Override standard method.
 */
void UMLView::closeEvent(QCloseEvent * e)
{
    QWidget::closeEvent(e);
}

#include "umlview.moc"
