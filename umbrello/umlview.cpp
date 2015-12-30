/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "umlview.h"

// application specific includes
#include "debug_utils.h"
#include "docwindow.h"
#include "model_utils.h"
#include "notewidget.h"
#include "uml.h"
#include "umldoc.h"
#include "umldragdata.h"
#include "umlscene.h"
#include "umlviewdialog.h"
#include "umlwidget.h"

#include <QPointer>

DEBUG_REGISTER(UMLView)

/**
 * Constructor.
 */
UMLView::UMLView(UMLFolder *parentFolder)
  : QGraphicsView(UMLApp::app()->mainViewWidget())
{
    setAcceptDrops(true);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setDragMode(NoDrag); //:TODO: RubberBandDrag);
    setScene(new UMLScene(parentFolder, this));
}

/**
 * Destructor.
 */
UMLView::~UMLView()
{
    delete umlScene();
}

/**
 * Getter for the uml scene.
 */
UMLScene* UMLView::umlScene() const
{
    return static_cast<UMLScene*>(scene());
}

/**
 * Returns the zoom of the diagram.
 */
int UMLView::zoom() const
{
    return (int)(matrix().m11()*100.0);
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
    setMatrix(wm);

    umlScene()->resizeSceneToItems();
}

/**
 * Shows the properties dialog for the view.
 */
bool UMLView::showPropertiesDialog(QWidget *parent)
{
    bool success = false;
    QPointer<UMLViewDialog> dlg = new UMLViewDialog(parent, umlScene());
    if (dlg->exec() == QDialog::Accepted) {
        success = true;
    }
    delete dlg;
    return success;
}

void UMLView::zoomIn()
{
    QMatrix wm = matrix();
    wm.scale(1.5, 1.5); // adjust zooming step here
    setZoom((int)(wm.m11()*100.0));
}

void UMLView::zoomOut()
{
    QMatrix wm = matrix();
    wm.scale(2.0 / 3.0, 2.0 / 3.0); //adjust zooming step here
    setZoom((int)(wm.m11()*100.0));
}

/**
 * Overrides standard method from QWidget to resize scene when
 * it's shown.
 */
void UMLView::show()
{
    QWidget::show();
    umlScene()->resizeSceneToItems();
}

/**
 * Zoom the view in and out.
 */
void UMLView::wheelEvent(QWheelEvent* event)
{
    // get the position of the mouse before scaling, in scene coords
    QPointF pointBeforeScale(mapToScene(event->pos()));

    // scale the view ie. do the zoom
    double scaleFactor = 1.15;
    if (event->delta() > 0) {
        // zoom in
        if (zoom() < 500) {
            scale(scaleFactor, scaleFactor);
        } else {
            return;
        }
    } else {
        // zooming out
        if (zoom() > 10) {
            scale(1.0 / scaleFactor, 1.0 / scaleFactor);
        } else {
            return;
        }
    }

    // get the position after scaling, in scene coords
    QPointF pointAfterScale(mapToScene(event->pos()));

    // get the offset of how the screen moved
    QPointF offset = pointBeforeScale - pointAfterScale;

    // adjust to the new center for correct zooming
    QPointF newCenter = mapToScene(rect()).boundingRect().center() + offset;
    centerOn(newCenter);

    DEBUG(DBG_SRC) << "currentZoom=" << zoom();
    UMLApp::app()->slotZoomSliderMoved(zoom());
}

/**
 * Overrides the standard operation.
 */
void UMLView::showEvent(QShowEvent* se)
{
    UMLApp* theApp = UMLApp::app();
    WorkToolBar* tb = theApp->workToolBar();
    UMLScene *us = umlScene();
    connect(tb, SIGNAL(sigButtonChanged(int)), us, SLOT(slotToolBarChanged(int)));
    connect(us, SIGNAL(sigResetToolBar()), tb, SLOT(slotResetToolBar()));

    umlScene()->showEvent(se);
    us->resetToolbar();
}

/**
 * Overrides the standard operation.
 */
void UMLView::hideEvent(QHideEvent* he)
{
    UMLApp* theApp = UMLApp::app();
    WorkToolBar* tb = theApp->workToolBar();
    UMLScene *us = umlScene();
    disconnect(tb, SIGNAL(sigButtonChanged(int)), us, SLOT(slotToolBarChanged(int)));
    disconnect(us, SIGNAL(sigResetToolBar()), tb, SLOT(slotResetToolBar()));

    us->hideEvent(he);
}

/**
 * Override standard method.
 */
void UMLView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MidButton) {
        setDragMode(QGraphicsView::ScrollHandDrag);
        setInteractive(false);
        QMouseEvent fake(event->type(), event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());
        QGraphicsView::mousePressEvent(&fake);
    } else
        QGraphicsView::mousePressEvent(event);
}

/**
 * Override standard method.
 */
void UMLView::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MidButton) {
        QMouseEvent fake(event->type(), event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());
        QGraphicsView::mouseReleaseEvent(&fake);
        setInteractive(true);
        setDragMode(QGraphicsView::NoDrag);
    } else
        QGraphicsView::mouseReleaseEvent(event);
}
