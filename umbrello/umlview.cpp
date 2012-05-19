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

// application specific includes
#include "docwindow.h"
#include "toolbarstatefactory.h"
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
    // Create the ToolBarState factory. This class is not a singleton, because it
    // needs a pointer to this object.
    m_pToolBarStateFactory = new ToolBarStateFactory();
    m_pToolBarState = m_pToolBarStateFactory->getState(WorkToolBar::tbb_Arrow, this->umlScene());
}

/**
 * Destructor
 */
UMLView::~UMLView()
{
    delete m_pToolBarStateFactory;
    m_pToolBarStateFactory = 0;
}

/**
 * Hack for reducing the difference
 * between the new QGraphicsScreen port.
 */
UMLScene* UMLView::umlScene() const
{
    return m_scene;
}

/**
 * Returns the zoom of the diagram.
 */
int UMLView::zoom() const
{
    return m_nZoom;
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
 * Return the current zoom factor.
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
void UMLView::closeEvent(QCloseEvent* ce)
{
    QWidget::closeEvent(ce);
}

/**
 * Overrides the standard operation.
 * Calls the same method in the current tool bar state.
 */
void UMLView::contentsMouseReleaseEvent(QMouseEvent* ome)
{
    m_pToolBarState->mouseRelease(static_cast<UMLSceneMouseEvent*>(ome));
}

/**
 * Overrides the standard operation.
 * Calls the same method in the current tool bar state.
 */
void UMLView::contentsMouseMoveEvent(QMouseEvent* ome)
{
    m_pToolBarState->mouseMove(static_cast<UMLSceneMouseEvent*>(ome));
}

/**
 * Override standard method.
 * Calls the same method in the current tool bar state.
 */
void UMLView::contentsMouseDoubleClickEvent(QMouseEvent* ome)
{
    m_pToolBarState->mouseDoubleClick(static_cast<UMLSceneMouseEvent*>(ome));
}


/**
 * Overrides the standard operation.
 */
void UMLView::showEvent(QShowEvent* se)
{
    UMLApp* theApp = UMLApp::app();
    WorkToolBar* tb = theApp->workToolBar();
    connect(tb, SIGNAL(sigButtonChanged(int)), this, SLOT(slotToolBarChanged(int)));
    connect(this, SIGNAL(sigResetToolBar()), tb, SLOT(slotResetToolBar()));

    umlScene()->showEvent(se);
    resetToolbar();
}

/**
 * Overrides the standard operation.
 */
void UMLView::hideEvent(QHideEvent* he)
{
    UMLApp* theApp = UMLApp::app();
    WorkToolBar* tb = theApp->workToolBar();
    disconnect(tb, SIGNAL(sigButtonChanged(int)), this, SLOT(slotToolBarChanged(int)));
    disconnect(this, SIGNAL(sigResetToolBar()), tb, SLOT(slotResetToolBar()));

    umlScene()->hideEvent(he);
}

/**
 * Override standard method.
 * Calls the same method in the current tool bar state.
 */
void UMLView::contentsMousePressEvent(QMouseEvent* ome)
{
    m_pToolBarState->mousePress(static_cast<UMLSceneMouseEvent*>(ome));
    //TODO should be managed by widgets when are selected. Right now also has some
    //problems, such as clicking on a widget, and clicking to move that widget shows
    //documentation of the diagram instead of keeping the widget documentation.
    //When should diagram documentation be shown? When clicking on an empty
    //space in the diagram with arrow tool?
    if (!m_bChildDisplayedDoc) {
        UMLApp::app()->docWindow()->showDocumentation(this, true);
    }
    m_bChildDisplayedDoc = false;
}

/**
 * Changes the current tool to the selected tool.
 * The current tool is cleaned and the selected tool initialized.
 */
void UMLView::slotToolBarChanged(int c)
{
    m_pToolBarState->cleanBeforeChange();
    m_pToolBarState = m_pToolBarStateFactory->getState((WorkToolBar::ToolBar_Buttons)c, umlScene());
    m_pToolBarState->init();

    umlScene()->setPaste(false);
    m_bChildDisplayedDoc = false;
}

/**
 *  Calls the same method in the DocWindow.
 */
void UMLView::showDocumentation(UMLObject * object, bool overwrite)
{
    UMLApp::app()->docWindow()->showDocumentation(object, overwrite);
    m_bChildDisplayedDoc = true;
}

/**
 *  Calls the same method in the DocWindow.
 */
void UMLView::showDocumentation(UMLWidget * widget, bool overwrite)
{
    UMLApp::app()->docWindow()->showDocumentation(widget, overwrite);
    m_bChildDisplayedDoc = true;
}

/**
 *  Calls the same method in the DocWindow.
 */
void UMLView::showDocumentation(AssociationWidget * widget, bool overwrite)
{
    UMLApp::app()->docWindow()->showDocumentation(widget, overwrite);
    m_bChildDisplayedDoc = true;
}

/**
 *  Calls the same method in the DocWindow.
 */
void UMLView::updateDocumentation(bool clear)
{
    UMLApp::app()->docWindow()->updateDocumentation(clear);
}

#include "umlview.moc"
