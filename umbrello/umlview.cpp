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
#include "debug_utils.h"
#include "docwindow.h"
#include "model_utils.h"
#include "notewidget.h"
#include "uml.h"
#include "umldoc.h"
#include "umldragdata.h"
#include "umlscene.h"
#include "umlwidget.h"

/**
 * Constructor
 */
UMLView::UMLView(UMLFolder *parentFolder)
  : QGraphicsView(UMLApp::app()->mainViewWidget()),
    m_nZoom(100)
{
    setAcceptDrops(true);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setDragMode(NoDrag); //:TODO: RubberBandDrag);
    // [PORT] For now the following is used. Shd check for creation of
    // new scene later.
    UMLScene *scene = new UMLScene(parentFolder, this);
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
 * Override standard method - Q3CanvasView specific.
 * Superceded by UMLScene::dragEnterEvent() - to be removed when transition
 * to QGraphicsView is complete
 */
void UMLView::dragEnterEvent(QDragEnterEvent *e)
{
    UMLDragData::LvTypeAndID_List tidList;
    if(!UMLDragData::getClip3TypeAndID(e->mimeData(), tidList)) {
        return;
    }
    UMLDragData::LvTypeAndID_It tidIt(tidList);
    if (!tidIt.hasNext()) {
        DEBUG(DBG_SRC) << "UMLDragData::getClip3TypeAndID returned empty list";
        return;
    }
    UMLDragData::LvTypeAndID * tid = tidIt.next();
    if (!tid) {
        uDebug() << "UMLView::contentsDragEnterEvent: "
                 << "UMLDragData::getClip3TypeAndID returned empty list";
        return;
    }
    UMLListViewItem::ListViewType lvtype = tid->type;
    Uml::IDType id = tid->id;

    Uml::DiagramType diagramType = umlScene()->type();

    UMLObject* temp = 0;
    //if dragging diagram - might be a drag-to-note
    if (Model_Utils::typeIsDiagram(lvtype)) {
        e->setAccepted(true);
        return;
    }
    //can't drag anything onto state/activity diagrams
    if( diagramType == Uml::DiagramType::State || diagramType == Uml::DiagramType::Activity) {
        e->setAccepted(false);
        return;
    }
    UMLDoc *pDoc = UMLApp::app()->document();
    //make sure can find UMLObject
    if( !(temp = pDoc->findObjectById(id) ) ) {
        kDebug() << "object " << ID2STR(id) << " not found" << endl;
        e->setAccepted(false);
        return;
    }
    //make sure dragging item onto correct diagram
    // concept - class,seq,coll diagram
    // actor,usecase - usecase diagram
    UMLObject::ObjectType ot = temp->baseType();
    bool bAccept = true;
    switch (diagramType) {
        case Uml::DiagramType::UseCase:
            if ((umlScene()->widgetOnDiagram(id) && ot == UMLObject::ot_Actor) ||
                (ot != UMLObject::ot_Actor && ot != UMLObject::ot_UseCase))
                bAccept = false;
            break;
        case Uml::DiagramType::Class:
            if (umlScene()->widgetOnDiagram(id) ||
                (ot != UMLObject::ot_Class &&
                 ot != UMLObject::ot_Package &&
                 ot != UMLObject::ot_Interface &&
                 ot != UMLObject::ot_Enum &&
                 ot != UMLObject::ot_Datatype)) {
                bAccept = false;
            }
            break;
        case Uml::DiagramType::Sequence:
        case Uml::DiagramType::Collaboration:
            if (ot != UMLObject::ot_Class &&
                ot != UMLObject::ot_Interface &&
                ot != UMLObject::ot_Actor)
                bAccept = false;
            break;
        case Uml::DiagramType::Deployment:
            if (umlScene()->widgetOnDiagram(id))
                bAccept = false;
            else if (ot != UMLObject::ot_Interface &&
                     ot != UMLObject::ot_Package &&
                     ot != UMLObject::ot_Component &&
                     ot != UMLObject::ot_Class &&
                     ot != UMLObject::ot_Node)
                bAccept = false;
            else if (ot == UMLObject::ot_Package &&
                     temp->stereotype() != "subsystem")
                bAccept = false;
            break;
        case Uml::DiagramType::Component:
            if (umlScene()->widgetOnDiagram(id) ||
                (ot != UMLObject::ot_Interface &&
                 ot != UMLObject::ot_Package &&
                 ot != UMLObject::ot_Component &&
                 ot != UMLObject::ot_Artifact &&
                 ot != UMLObject::ot_Class))
                bAccept = false;
            if (ot == UMLObject::ot_Class && !temp->isAbstract())
                bAccept = false;
            break;
        case Uml::DiagramType::EntityRelationship:
            if (ot != UMLObject::ot_Entity)
                bAccept = false;
            break;
        default:
            break;
    }
    e->setAccepted(bAccept);
}

/**
 * Override standard method - Q3CanvasView specific.
 * Superceded by UMLScene::dropEvent() - to be removed when transition to
 * QGraphicsView is complete.
 */
void UMLView::dropEvent(QDropEvent *e) {
    UMLDragData::LvTypeAndID_List tidList;
    if( !UMLDragData::getClip3TypeAndID(e->mimeData(), tidList) ) {
        return;
    }
    UMLDragData::LvTypeAndID_It tidIt(tidList);
    if (!tidIt.hasNext()) {
        DEBUG(DBG_SRC) << "UMLDragData::getClip3TypeAndID returned empty list";
        return;
    }
    UMLDragData::LvTypeAndID * tid = tidIt.next();
    if (!tid) {
        kDebug() << "UMLView::contentsDropEvent: "
                  << "UMLDragData::getClip3TypeAndID returned empty list" << endl;
        return;
    }
    UMLListViewItem::ListViewType lvtype = tid->type;
    Uml::IDType id = tid->id;

    if (Model_Utils::typeIsDiagram(lvtype)) {
        bool breakFlag = false;
        UMLWidgetList widgets = umlScene()->widgetList();
        UMLWidget* w = 0;
        foreach (w, widgets) {
            if (w->baseType() == WidgetBase::wt_Note && w->onWidget(mapToScene(e->pos()))) {
                breakFlag = true;
                break;
            }
        }
        if (breakFlag) {
            NoteWidget *note = static_cast<NoteWidget*>(w);
            note->setDiagramLink(id);
        }
        return;
    }
    UMLDoc *pDoc = UMLApp::app()->document();
    UMLObject* o = pDoc->findObjectById(id);
    if( !o ) {
        kDebug() << "UMLView::contentsDropEvent: object id=" << ID2STR(id)
                  << " not found" << endl;
        return;
    }
    umlScene()->setCreateObject(true);
    umlScene()->setPos( (mapToScene(e->pos()) * 100 ) / m_nZoom );

    umlScene()->slotObjectCreated(o);

    pDoc -> setModified(true);
}

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
