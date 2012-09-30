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

#include "debug_utils.h"
#include "docwindow.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "worktoolbar.h"

#include <QMouseEvent>

/**
 * Constructor
 */
UMLView::UMLView(UMLFolder *parentFolder)
  : QGraphicsView(UMLApp::app()->mainViewWidget()),
    m_nZoom(100)
{
    setAcceptDrops(true);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setDragMode(RubberBandDrag);
    // [PORT] For now the following is used. Shd check for creation of
    // new scene later.
    UMLScene *scene = new UMLScene(parentFolder);
    setScene(scene);
    setSceneRect(scene->sceneRect());

    DEBUG_REGISTER(DBG_SRC);
}

/**
 * Destructor.
 */
UMLView::~UMLView()
{
    //TODO: Check if the scene shd be deleted
}

/**
 * Getter for the scene.
 * TODO: Should be removed. Use scene() instead.
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
    setMatrix(wm);

    m_nZoom = currentZoom();
    umlScene()->resizeCanvasToItems();
}

/**
 * Return the current zoom factor.
 */
int UMLView::currentZoom()
{
    return (int)(matrix().m11()*100.0);
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
 * Overrides standard method from QWidget to resize canvas when
 * it's shown.
 */
void UMLView::show()
{
    QWidget::show();
    umlScene()->resizeCanvasToItems();
}

/**
 * Handles when the mouse button is pressed.
 */
/*:TODO:
void UMLView::mousePressEvent(QMouseEvent* event)
{
    // for panning the view
    m_lastPanPoint = event->pos();
}
*/

/**
 * Handles when the mouse button is released.
 */
/*:TODO:
void UMLView::mouseReleaseEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    m_lastPanPoint = QPoint();
}
*/

/**
 * Handles the mouse move event.
 */
/*:TODO:
void UMLView::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_lastPanPoint.isNull()) {
        // get how much we panned
        QPointF delta = mapToScene(m_lastPanPoint) - mapToScene(event->pos());
        m_lastPanPoint = event->pos();

        // update the center ie. do the pan
        setCenter(center() + delta);
    }
}
*/

/**
 * Zoom the view in and out.
 */
void UMLView::wheelEvent(QWheelEvent* event)
{
    // get the position of the mouse before scaling, in scene coords
    QPointF pointBeforeScale(mapToScene(event->pos()));

    // get the original screen centerpoint
    QPointF screenCenter = center();

    // scale the view ie. do the zoom
    double scaleFactor = 1.15;
    if (event->delta() > 0) {
        // zoom in
        if (currentZoom() < 500) {
            scale(scaleFactor, scaleFactor);
        }
    } else {
        // zooming out
        if (currentZoom() > 10) {
            scale(1.0 / scaleFactor, 1.0 / scaleFactor);
        }
    }

    // get the position after scaling, in scene coords
    QPointF pointAfterScale(mapToScene(event->pos()));

    // get the offset of how the screen moved
    QPointF offset = pointBeforeScale - pointAfterScale;

    // adjust to the new center for correct zooming
    QPointF newCenter = screenCenter + offset;
    setCenter(newCenter);

    DEBUG(DBG_SRC) << "currentZoom=" << currentZoom();
    UMLApp::app()->slotZoomSliderMoved(currentZoom());
}

/**
 * Need to update the center so there is no jolt in the
 * interaction after resizing the widget.
 */
void UMLView::resizeEvent(QResizeEvent* event)
{
    // get the rectangle of the visible area in scene coords
    QRectF visibleArea = mapToScene(rect()).boundingRect();
    setCenter(visibleArea.center());

    // call the subclass resize so the scrollbars are updated correctly
    QGraphicsView::resizeEvent(event);
}

/**
 * Event handler for the show event.
 */
void UMLView::showEvent(QShowEvent* /*se*/)
{
    UMLApp* theApp = UMLApp::app();
    UMLDoc *doc = theApp->document();
    WorkToolBar* tb = theApp->workToolBar();
    UMLScene *us = umlScene();
    connect(tb, SIGNAL(sigButtonChanged(int)), us, SLOT(slotToolBarChanged(int)));
    connect(us, SIGNAL(sigResetToolBar()), tb, SLOT(slotResetToolBar()));
    connect(doc, SIGNAL(sigObjectCreated(UMLObject*)),
            us, SLOT(slotObjectCreated(UMLObject*)));
    connect(us, SIGNAL(sigAssociationRemoved(AssociationWidget*)),
            UMLApp::app()->docWindow(), SLOT(slotAssociationRemoved(AssociationWidget*)));
    connect(us, SIGNAL(sigWidgetRemoved(UMLWidget*)),
            UMLApp::app()->docWindow(), SLOT(slotWidgetRemoved(UMLWidget*)));
    us->resetToolbar();

}

/**
 * Event handler for the hide event.
 */
void UMLView::hideEvent(QHideEvent* /*he*/)
{
    UMLApp* theApp = UMLApp::app();
    UMLDoc *doc = theApp->document();
    WorkToolBar* tb = theApp->workToolBar();
    UMLScene *us = umlScene();
    disconnect(tb, SIGNAL(sigButtonChanged(int)), us, SLOT(slotToolBarChanged(int)));
    disconnect(us, SIGNAL(sigResetToolBar()), tb, SLOT(slotResetToolBar()));
    disconnect(doc, SIGNAL(sigObjectCreated(UMLObject *)), us, SLOT(slotObjectCreated(UMLObject *)));
    disconnect(us, SIGNAL(sigAssociationRemoved(AssociationWidget*)),
               UMLApp::app()->docWindow(), SLOT(slotAssociationRemoved(AssociationWidget*)));
    disconnect(us, SIGNAL(sigWidgetRemoved(UMLWidget*)),
               UMLApp::app()->docWindow(), SLOT(slotWidgetRemoved(UMLWidget*)));
}

/**
 * Override standard method.
 */
void UMLView::closeEvent(QCloseEvent* ce)
{
    QWidget::closeEvent(ce);
}

/**
 * Sets the current centerpoint.  Also updates the scene's center point.
 * Unlike centerOn, which has no way of getting the floating point center
 * back, setCenter() stores the center point.  It also handles the special
 * sidebar case.  This function will claim the centerPoint to sceneRec ie.
 * the centerPoint must be within the sceneRec.
 */
void UMLView::setCenter(const QPointF& centerPoint)
{
    // get the rectangle of the visible area in scene coords
    QRectF visibleArea = mapToScene(rect()).boundingRect();

    // get the scene area
    QRectF sceneBounds = sceneRect();

    double boundX = visibleArea.width() / 2.0;
    double boundY = visibleArea.height() / 2.0;
    double boundWidth = sceneBounds.width() - 2.0 * boundX;
    double boundHeight = sceneBounds.height() - 2.0 * boundY;

    // the max boundary that the centerPoint can be to
    QRectF bounds(boundX, boundY, boundWidth, boundHeight);

    if (bounds.contains(centerPoint)) {
        // we are within the bounds
        m_currentCenterPoint = centerPoint;
    } else {
        // we need to clamp or use the center of the screen
        if(visibleArea.contains(sceneBounds)) {
            // use the center of scene ie. we can see the whole scene
            m_currentCenterPoint = sceneBounds.center();
        } else {

            m_currentCenterPoint = centerPoint;

            // we need to clamp the center. The centerPoint is too large
            if (centerPoint.x() > bounds.x() + bounds.width()) {
                m_currentCenterPoint.setX(bounds.x() + bounds.width());
            } else if (centerPoint.x() < bounds.x()) {
                m_currentCenterPoint.setX(bounds.x());
            }

            if (centerPoint.y() > bounds.y() + bounds.height()) {
                m_currentCenterPoint.setY(bounds.y() + bounds.height());
            } else if (centerPoint.y() < bounds.y()) {
                m_currentCenterPoint.setY(bounds.y());
            }

        }
    }
    // update the scrollbars
    centerOn(m_currentCenterPoint);
}

/**
 * Get the center.
 */
QPointF UMLView::center()
{
    return m_currentCenterPoint;
}

#include "umlview.moc"
