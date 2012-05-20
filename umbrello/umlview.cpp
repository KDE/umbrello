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
#include "model_utils.h"
#include "umldoc.h"
#include "umlwidget.h"
#include "notewidget.h"
#include "umldragdata.h"
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
 * Override standard method - Q3CanvasView specific.
 * Superceded by UMLScene::dragEnterEvent() - to be removed when transition
 * to QGraphicsView is complete
 */
void UMLView::contentsDragEnterEvent(QDragEnterEvent *e) {
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
        kDebug() << "UMLView::contentsDragEnterEvent: "
                  << "UMLDragData::getClip3TypeAndID returned empty list" << endl;
        return;
    }
    UMLListViewItem::ListViewType lvtype = tid->type;
    Uml::IDType id = tid->id;

    Uml::DiagramType diagramType = umlScene()->type();

    UMLObject* temp = 0;
    //if dragging diagram - might be a drag-to-note
    if (Model_Utils::typeIsDiagram(lvtype)) {
        e->accept(true);
        return;
    }
    //can't drag anything onto state/activity diagrams
    if( diagramType == Uml::DiagramType::State || diagramType == Uml::DiagramType::Activity) {
        e->accept(false);
        return;
    }
    UMLDoc *pDoc = UMLApp::app()->document();
    //make sure can find UMLObject
    if( !(temp = pDoc->findObjectById(id) ) ) {
        kDebug() << "object " << ID2STR(id) << " not found" << endl;
        e->accept(false);
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
    e->accept(bAccept);
}

/**
 * Override standard method - Q3CanvasView specific.
 * Superceded by UMLScene::dropEvent() - to be removed when transition to
 * QGraphicsView is complete.
 */
void UMLView::contentsDropEvent(QDropEvent *e) {
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
            if (w->baseType() == WidgetBase::wt_Note && w->onWidget(e->pos())) {
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
    umlScene()->setPos( (e->pos() * 100 ) / m_nZoom );

    umlScene()->slotObjectCreated(o);

    pDoc -> setModified(true);
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
