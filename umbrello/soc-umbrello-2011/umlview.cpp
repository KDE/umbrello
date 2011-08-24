/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "umlview.h"
#include "folder.h"
#include "diagram.h"
#include "umlscene.h"
#include "uml.h"
#include "debug_utils.h"
#include "docwindow.h"
#include "umldoc.h"
#include "worktoolbar.h"
#include "ktabwidget.h"
#include <QtGui/QMouseEvent>
#include "umldoc.h"
#include "umlobject.h"
#include "widgetbase.h"
#include <kinputdialog.h>
#include "object_factory.h"
#include "umlviewimageexporter.h"

#include <QGraphicsView>
#include <KInputDialog>
#include <klocalizedstring.h>
#include <umlview.h>


namespace QGV {

UMLView::UMLView( UMLFolder *parentFolder)
: QGraphicsView(UMLApp::app()->mainViewWidget()),
m_folder(parentFolder),
m_doc(UMLApp::app()->document()),
m_scene(new UMLScene(m_folder, this)),
m_bShowSnapGrid(false),
m_bUseSnapToGrid(false)
{
 
 m_diagram = new Diagram(this);
 setDiagram(m_diagram);
 m_zoom = 100;
 setAcceptDrops(true);
 setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
 setDragMode(RubberBandDrag);
 setCacheMode(CacheNone);
 setRubberBandSelectionMode(Qt::IntersectsItemShape);
 setScene(m_scene);
 show(); 
   
}

UMLView::~UMLView()
{
}


UMLFolder *UMLView::folder() const
{
  return m_folder;
}

Diagram *UMLView::diagram()
{
  if(m_diagram == NULL)
  {
    m_diagram = new Diagram(this);
    setDiagram(m_diagram);
    if(m_diagram != NULL)
      qDebug() << "m_diagram is not NULL anymore";
  }else{
    qDebug() << "m_diagram is not NULL";
  }
    
  return m_diagram;
}

qreal UMLView::zoom() const
{
  return m_zoom;
}

qreal UMLView::currentZoom()
{
   return (transform().m11()*100.0);
}

void UMLView::setZoom(qreal zoom)
{
    if (zoom < 10) {
        zoom = 10;
    } else if (zoom > 500) {
        zoom = 500;
    }

    QTransform wm;
    wm.scale(zoom / 100.0, zoom / 100.0);    
    setTransform(wm);

    m_zoom = currentZoom();
  
}

void UMLView::zoomIn()
{
    QTransform wm = transform();
    wm.scale(1.5, 1.5); // adjust zooming step here
    setZoom((wm.m11()*100.0));
}

void UMLView::zoomOut()
{
    QTransform wm = transform();
    wm.scale(2.0 / 3.0, 2.0 / 3.0); //adjust zooming step here
    setZoom(wm.m11()*100.0);
}


void UMLView::setFolder(UMLFolder* folder)
{
  m_folder = folder;
}

void UMLView::setDiagram(Diagram* diagram)
{
  m_diagram = diagram;
}


UMLScene* UMLView::scene() const
{
  return m_scene;
}


void UMLView::setCenter(const QPointF& centerPoint)
{
    // get the rectangle of the visible area in scene coords
    //Returns the viewport coordinate point mapped to scene coordinates.
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

QPointF UMLView::center() const
{
  return m_currentCenterPoint;
}

void UMLView::showEvent(QShowEvent* /*se*/)
{
    UMLApp *theApp = UMLApp::app();
    UMLDoc *doc = theApp->document();
    WorkToolBar *toolbar = theApp->workToolBar();
    UMLScene *umlscene = scene();
    connect(toolbar, SIGNAL(sigButtonChanged(int)), umlscene, SLOT(slotToolBarChanged(int)));
    connect(umlscene, SIGNAL(sigResetToolBar()), toolbar, SLOT(slotResetToolBar()));
    connect(doc, SIGNAL(sigObjectCreated(UMLObject *)), m_diagram, SLOT(slotObjectCreated(UMLObject *)));
    //connect(umlscene, SIGNAL(sigAssociationRemoved(AssociationWidget*)), UMLApp::app()->docWindow(), SLOT(slotAssociationRemoved(AssociationWidget*)));
    //connect(umlscene, SIGNAL(sigWidgetRemoved(UMLWidget*)), UMLApp::app()->docWindow(), SLOT(slotWidgetRemoved(UMLWidget*))); 
    umlscene->resetToolbar();

}

void UMLView::closeEvent(QCloseEvent * e)
{
    QWidget::closeEvent(e);
}

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

    //DEBUG(DBG_SRC) << "currentZoom=" << currentZoom();
}

void UMLView::hideEvent(QHideEvent* /*he*/)
{
    UMLApp *theApp = UMLApp::app();
    UMLDoc *doc = theApp->document();
    WorkToolBar *toolbar = theApp->workToolBar();
    //new canvas
    UMLScene *umlscene = scene();
    //disconnect(toolbar, SIGNAL(sigButtonChanged(int)), umlscene, SLOT(slotToolBarChanged(int)));
    //disconnect(umlscene, SIGNAL(sigResetToolBar()), toolbar, SLOT(slotResetToolBar()));
    //disconnect(doc, SIGNAL(sigObjectCreated(UMLObject *)), umlscene, SLOT(slotObjectCreated(UMLObject *)));
    //disconnect(umlscene, SIGNAL(sigAssociationRemoved(AssociationWidget*)),
               //UMLApp::app()->docWindow(), SLOT(slotAssociationRemoved(AssociationWidget*)));
    //disconnect(umlscene, SIGNAL(sigWidgetRemoved(UMLWidget*)),
               //UMLApp::app()->docWindow(), SLOT(slotWidgetRemoved(UMLWidget*)));
}

void UMLView::slotShowView()
{
    m_doc->changeCurrentView(diagram()->id());
}

void UMLView::slotActivate()
{
    m_doc->changeCurrentView(diagram()->id());
}


void UMLView::resizeEvent(QResizeEvent* event)
{
    // get the rectangle of the visible area in scene coords
    QRectF visibleArea = mapToScene(rect()).boundingRect();
    setCenter(visibleArea.center());

    // call the subclass resize so the scrollbars are updated correctly
    QGraphicsView::resizeEvent(event);
}

UMLDoc* UMLView::doc() const
{
  return m_doc;
}

void UMLView::setDoc(UMLDoc* doc)
{
  m_doc = doc;
}

void UMLView::setMenu(QPoint pos)
{
    //slotRemovePopupMenu();
  
    ListPopupMenu::MenuType menu = ListPopupMenu::mt_Undefined;
    
    switch (UMLApp::app()->currentView()->type()) {
      case DiagramType::Class:
        menu = ListPopupMenu::mt_On_Class_Diagram;
        break;

    case DiagramType::UseCase:
        menu = ListPopupMenu::mt_On_UseCase_Diagram;
        break;

    case DiagramType::Sequence:
        menu = ListPopupMenu::mt_On_Sequence_Diagram;
        break;

    case DiagramType::Collaboration:
        menu = ListPopupMenu::mt_On_Collaboration_Diagram;
        break;

    case DiagramType::State:
        menu = ListPopupMenu::mt_On_State_Diagram;
        break;

    case DiagramType::Activity:
        menu = ListPopupMenu::mt_On_Activity_Diagram;
        break;

    case DiagramType::Component:
        menu = ListPopupMenu::mt_On_Component_Diagram;
        break;

    case DiagramType::Deployment:
        menu = ListPopupMenu::mt_On_Deployment_Diagram;
        break;

    case DiagramType::EntityRelationship:
        menu = ListPopupMenu::mt_On_EntityRelationship_Diagram;
        break;

    default:
        uWarning() << "unknown new diagram type " << m_diagram->typeDiagram();
        menu = ListPopupMenu::mt_Undefined;
        break;
    }//end switch
    //if (menu != ListPopupMenu::mt_Undefined) {
    if (menu != ListPopupMenu::mt_Undefined) {
        // uDebug() << "create popup for Menu_Type " << menu;
        m_menu = new ListPopupMenu(UMLApp::app()->current_View(), menu, UMLApp::app()->current_View());
        connect(m_menu, SIGNAL(triggered(QAction*)), this, SLOT(slotMenuSelection(QAction*)));
        //m_menu->popup(mapToGlobal(QPoint((int)m_pos.x(), (int)m_pos.y())));
	qDebug() << "new method setMenu here!";
	m_menu->popup(pos);
    }else{
	qDebug() << "menu == ListPopupMenu::mt_Undefined";
    }
}

void UMLView::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = ListPopupMenu::mt_Undefined;
    if (m_menu != NULL) {  // popup from this class
        sel = m_menu->getMenuType(action);
    } else { // popup from umldoc
        sel = m_doc->popupMenuSelection(action);
    }
    switch (sel) {
    case ListPopupMenu::mt_Undo:
        UMLApp::app()->undo();
        break;

    case ListPopupMenu::mt_Redo:
        UMLApp::app()->redo();
        break;

    case ListPopupMenu::mt_Clear:
//         clearDiagram();
        break;

    case ListPopupMenu::mt_Export_Image:
        scene()->imageExporter()->exportView();
        break;

    case ListPopupMenu::mt_FloatText: {
//         FloatingTextWidget* ft = new FloatingTextWidget(this);
//         ft->showChangeTextDialog();
//         //if no text entered delete
//         if (!FloatingTextWidget::isTextValid(ft->text())) {
//             delete ft;
//         } else {
//             ft->setID(UniqueID::gen());
//             setupNewWidget(ft);
//         }
    }
    break;

    case ListPopupMenu::mt_UseCase:
//         m_bCreateObject = true;
        Object_Factory::createUMLObject(UMLObject::ot_UseCase);
        break;

    case ListPopupMenu::mt_Actor:
//         m_bCreateObject = true;
        Object_Factory::createUMLObject(UMLObject::ot_Actor);
        break;

    case ListPopupMenu::mt_Class:
    case ListPopupMenu::mt_Object:
//         m_bCreateObject = true;
        Object_Factory::createUMLObject(UMLObject::ot_Class);
	Object_Factory::createUMLObject(UMLObject::ot_Class);
        break;

    case ListPopupMenu::mt_Package:
//         m_bCreateObject = true;
        Object_Factory::createUMLObject(UMLObject::ot_Package);
        break;

    case ListPopupMenu::mt_Component:
//         m_bCreateObject = true;
        Object_Factory::createUMLObject(UMLObject::ot_Component);
        break;

    case ListPopupMenu::mt_Node:
//         m_bCreateObject = true;
        Object_Factory::createUMLObject(UMLObject::ot_Node);
        break;

    case ListPopupMenu::mt_Artifact:
//         m_bCreateObject = true;
        Object_Factory::createUMLObject(UMLObject::ot_Artifact);
        break;

    case ListPopupMenu::mt_Interface:
//         m_bCreateObject = true;
        Object_Factory::createUMLObject(UMLObject::ot_Interface);
        break;

    case ListPopupMenu::mt_Enum:
//         m_bCreateObject = true;
        Object_Factory::createUMLObject(UMLObject::ot_Enum);
        break;

    case ListPopupMenu::mt_Entity:
//         m_bCreateObject = true;
        Object_Factory::createUMLObject(UMLObject::ot_Entity);
        break;

    case ListPopupMenu::mt_Category:
//         m_bCreateObject = true;
        Object_Factory::createUMLObject(UMLObject::ot_Category);
        break;

    case ListPopupMenu::mt_Datatype:
//         m_bCreateObject = true;
        Object_Factory::createUMLObject(UMLObject::ot_Datatype);
        break;

    case ListPopupMenu::mt_Cut:
        //FIXME make this work for diagram's right click menu
        if (scene()->selectedItems().count() &&
                UMLApp::app()->editCutCopy(true)) {
            scene()->selectedItems().clear();
            m_doc->setModified(true);
        }
        break;

    case ListPopupMenu::mt_Copy:
        //FIXME make this work for diagram's right click menu
        scene()->selectedItems().count() && UMLApp::app()->editCutCopy(true);
        break;

    case ListPopupMenu::mt_Paste:
        m_pastepoint = m_pos;
        m_pos.setX(2000);
        m_pos.setY(2000);
        UMLApp::app()->slotEditPaste();

        m_pastepoint.setX(0);
        m_pastepoint.setY(0);
        break;

    case ListPopupMenu::mt_Initial_State: {
//         StateWidget* state = new StateWidget(this, StateWidget::Initial);
//         setupNewWidget(state);
    }
    break;

    case ListPopupMenu::mt_End_State: {
//         StateWidget* state = new StateWidget(this, StateWidget::End);
//         setupNewWidget(state);
    }
    break;

    case ListPopupMenu::mt_State: {
        bool ok = false;
        QString name = KInputDialog::getText(i18n("Enter State Name"),
                                             i18n("Enter the name of the new state:"),
                                             i18n("new state"), &ok, UMLApp::app());
        if (ok) {
//             StateWidget* state = new StateWidget(this);
//             state->setName(name);
//             setupNewWidget(state);
        }
    }
    break;

    case ListPopupMenu::mt_Initial_Activity: {
//         ActivityWidget* activity = new ActivityWidget(this, ActivityWidget::Initial);
//         setupNewWidget(activity);
    }
    break;


    case ListPopupMenu::mt_End_Activity: {
//         ActivityWidget* activity = new ActivityWidget(this, ActivityWidget::End);
//         setupNewWidget(activity);
    }
    break;

    case ListPopupMenu::mt_Branch: {
//         ActivityWidget* activity = new ActivityWidget(this, ActivityWidget::Branch);
//         setupNewWidget(activity);
    }
    break;

    case ListPopupMenu::mt_Activity: {
        bool ok = false;
        QString name = KInputDialog::getText(i18n("Enter Activity Name"),
                                             i18n("Enter the name of the new activity:"),
                                             i18n("new activity"), &ok, UMLApp::app());
        if (ok) {
//             ActivityWidget* activity = new ActivityWidget(this, ActivityWidget::Normal);
//             activity->setName(name);
//             setupNewWidget(activity);
        }
    }
    break;

    case ListPopupMenu::mt_SnapToGrid:
        toggleSnapToGrid();
        m_doc->setModified();
        break;

    case ListPopupMenu::mt_ShowSnapGrid:
	toggleShowGrid();
        m_doc->setModified();
        break;

    case ListPopupMenu::mt_Properties:
         if (scene()->showPropDialog() == true)
            m_doc->setModified();
        break;

    case ListPopupMenu::mt_Delete:
        m_doc->removeDiagram(m_diagram->id());
        break;

    case ListPopupMenu::mt_Rename: {
        bool ok = false;
        QString name = KInputDialog::getText(i18n("Enter Diagram Name"),
                                             i18n("Enter the new name of the diagram:"),
                                             m_diagram->name(), &ok, UMLApp::app());
        if (ok) {
            m_diagram->setName(name);
            m_doc->signalDiagramRenamed(this);
        }
    }
    break;

    default:
        uWarning() << "unknown NEW ListPopupMenu::Menu_Type " << sel;
        break;
    }
 
}

void UMLView::toggleShowGrid()
{
    setShowSnapGrid(!snapToGrid());
}

bool UMLView::snapToGrid()
{
  return m_bShowSnapGrid;
}


void UMLView::setSnapToGrid(bool bSnap)
{
    m_bUseSnapToGrid = bSnap;
    emit sigSnapToGridToggled(m_scene->snapToGrid());
}

void UMLView::setShowSnapGrid(bool bShow)
{
    m_bShowSnapGrid = bShow;
    emit sigShowGridToggled(snapToGrid());
}

void UMLView::setSnapGridVisible(bool bShow)
{
    scene()->layoutGrid()->setVisible(bShow);
    emit sigShowGridToggled(bShow);  
}

void UMLView::toggleSnapToGrid()
{
    setSnapToGrid(!snapToGrid());
}

}


