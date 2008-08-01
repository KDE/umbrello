/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "toolbarstate.h"

// qt includes
#include <qwmatrix.h> // need for inverseWorldMatrix.map

// app includes
#include "associationwidget.h"
#include "messagewidget.h"
#include "floatingdashlinewidget.h"
#include "objectwidget.h"
#include "uml.h"
#include "umlscene.h"
#include "umlwidget.h"

ToolBarState::~ToolBarState() {
    delete m_pMouseEvent;
}

void ToolBarState::init() {
    // [PORT]
    // m_pUMLScene->viewport()->setMouseTracking(false);
    m_pMouseEvent = 0;
    m_currentWidget = 0;
    m_currentAssociation = 0;

    connect(m_pUMLScene, SIGNAL(sigAssociationRemoved(AssociationWidget*)),
            this, SLOT(slotAssociationRemoved(AssociationWidget*)));
    connect(m_pUMLScene, SIGNAL(sigWidgetRemoved(NewUMLRectWidget*)),
            this, SLOT(slotWidgetRemoved(NewUMLRectWidget*)));
}

void ToolBarState::cleanBeforeChange() {
    disconnect(m_pUMLScene, SIGNAL(sigAssociationRemoved(AssociationWidget*)),
               this, SLOT(slotAssociationRemoved(AssociationWidget*)));
    disconnect(m_pUMLScene, SIGNAL(sigWidgetRemoved(NewUMLRectWidget*)),
               this, SLOT(slotWidgetRemoved(NewUMLRectWidget*)));
}

void ToolBarState::mousePress(QGraphicsSceneMouseEvent* ome) {
    setMouseEvent(ome, QEvent::MouseButtonPress);

    // [PORT]
    // m_pUMLScene->viewport()->setMouseTracking(true);

    //TODO Doesn't another way of emiting the signal exist? A method only for
    //that seems a bit dirty.
    m_pUMLScene->emitRemovePopupMenu();

    // TODO: Check who needs this.
    m_pUMLScene->setPos(m_pMouseEvent->scenePos());

    //TODO check why
    m_pUMLScene->setPaste(false);

    setCurrentElement();

    if (getCurrentWidget()) {
        mousePressWidget();
    } else if (getCurrentAssociation()) {
        mousePressAssociation();
    } else {
        mousePressEmpty();
    }
}

void ToolBarState::mouseRelease(QGraphicsSceneMouseEvent* ome) {
    setMouseEvent(ome, QEvent::MouseButtonRelease);

    // Set the position of the mouse
    // TODO, should only be available in this state?
// [PORT] Check if scenePos fits in here
    m_pUMLScene->setPos(m_pMouseEvent->scenePos());

    // [PORT]
    // m_pUMLScene->viewport()->setMouseTracking(false);

    if (getCurrentWidget()) {
        mouseReleaseWidget();
        setCurrentWidget(0);
    } else if (getCurrentAssociation()) {
        mouseReleaseAssociation();
        setCurrentAssociation(0);
    } else {
        mouseReleaseEmpty();
    }

    // Default, rightbutton changes the tool.
    // The arrow tool overrides the changeTool() function.
    changeTool();
}

void ToolBarState::mouseDoubleClick(QGraphicsSceneMouseEvent* ome) {
    setMouseEvent(ome, QEvent::MouseButtonDblClick);

// [PORT] Check if scenePos fits in here
    NewUMLRectWidget* currentWidget = m_pUMLScene->getWidgetAt(m_pMouseEvent->scenePos());
// [PORT] Check if scenePos fits in here
    AssociationWidget* currentAssociation = getAssociationAt(m_pMouseEvent->scenePos());
    if (currentWidget) {
        setCurrentWidget(currentWidget);
        mouseDoubleClickWidget();
        setCurrentWidget(0);
    } else if (currentAssociation) {
        setCurrentAssociation(currentAssociation);
        mouseDoubleClickAssociation();
        setCurrentAssociation(0);
    } else {
        mouseDoubleClickEmpty();
    }
}

void ToolBarState::mouseMove(QGraphicsSceneMouseEvent* ome) {
    setMouseEvent(ome, QEvent::MouseMove);

    if (getCurrentWidget()) {
        mouseMoveWidget();
    } else if (getCurrentAssociation()) {
        mouseMoveAssociation();
    } else {
        mouseMoveEmpty();
    }

    // [PORT]
#if 0
    //Scrolls the view
    int vx = ome->x();
    int vy = ome->y();
    int contsX = m_pUMLScene->contentsX();
    int contsY = m_pUMLScene->contentsY();
    int visw = m_pUMLScene->visibleWidth();
    int vish = m_pUMLScene->visibleHeight();
    int dtr = visw - (vx-contsX);
    int dtb = vish - (vy-contsY);
    int dtt =  (vy-contsY);
    int dtl =  (vx-contsX);
    if (dtr < 30) m_pUMLScene->scrollBy(30-dtr,0);
    if (dtb < 30) m_pUMLScene->scrollBy(0,30-dtb);
    if (dtl < 30) m_pUMLScene->scrollBy(-(30-dtl),0);
    if (dtt < 30) m_pUMLScene->scrollBy(0,-(30-dtt));
#endif
}

void ToolBarState::slotAssociationRemoved(AssociationWidget* association) {
    if (association == getCurrentAssociation()) {
        setCurrentAssociation(0);
    }
}

void ToolBarState::slotWidgetRemoved(NewUMLRectWidget* widget) {
    if (widget == getCurrentWidget()) {
        setCurrentWidget(0);
    }
}

ToolBarState::ToolBarState(UMLScene *umlScene) : QObject(umlScene),
                                                 m_pUMLScene(umlScene)
{
    m_pMouseEvent = NULL;
    init();
}

void ToolBarState::setCurrentElement()
{
    // Check associations.
// [PORT] Check if scenePos fits in here
    AssociationWidget* association = getAssociationAt(m_pMouseEvent->scenePos());
    if (association) {
        setCurrentAssociation(association);
        return;
    }

    // Check messages.
    //TODO check why message widgets are treated different
// [PORT] Check if scenePos fits in here
    MessageWidget* message = getMessageAt(m_pMouseEvent->scenePos());
    if (message) {
        setCurrentWidget(message);
        return;
    }

    //TODO check why message widgets are treated different
// [PORT] Check if scenePos fits in here
    FloatingDashLineWidget* floatingline = getFloatingLineAt(m_pMouseEvent->scenePos());
    if (floatingline) {
        setCurrentWidget(floatingline);
        return;
    }


// [PORT] Check if scenePos fits in here
    ObjectWidget* objectWidgetLine = m_pUMLScene->onWidgetDestructionBox(m_pMouseEvent->scenePos());
    if (objectWidgetLine) {
        setCurrentWidget(objectWidgetLine);
        return;
    }

    // Check widgets.
// [PORT] Check if scenePos fits in here
    NewUMLRectWidget *widget = m_pUMLScene->getWidgetAt(m_pMouseEvent->scenePos());
    if (widget) {
        setCurrentWidget(widget);
        return;
    }
}

void ToolBarState::mousePressAssociation() {
}

void ToolBarState::mousePressWidget() {
}

void ToolBarState::mousePressEmpty() {
    m_pUMLScene->clearSelected();
}

void ToolBarState::mouseReleaseAssociation() {
}

void ToolBarState::mouseReleaseWidget() {
}

void ToolBarState::mouseReleaseEmpty() {
}

void ToolBarState::mouseDoubleClickAssociation() {
}

void ToolBarState::mouseDoubleClickWidget() {
}

void ToolBarState::mouseDoubleClickEmpty() {
    m_pUMLScene->clearSelected();
}

void ToolBarState::mouseMoveAssociation() {
}

void ToolBarState::mouseMoveWidget() {
}

void ToolBarState::mouseMoveEmpty() {
}

void ToolBarState::changeTool() {
    // [PORT] Check if button works well instead of state()
    if (m_pMouseEvent->button() == Qt::RightButton) {
        UMLApp::app()->getWorkToolBar()->setDefaultTool();
    }
}

void ToolBarState::setMouseEvent(QGraphicsSceneMouseEvent* ome, const QEvent::Type &type)
{
    delete m_pMouseEvent;

    // [PORT] Check if scenePos works like view->inverseWorldMatrix().map()
    // Using copy constructor here.
    m_pMouseEvent = new QGraphicsSceneMouseEvent(type);
    m_pMouseEvent->setPos(ome->pos());
    m_pMouseEvent->setScenePos(ome->scenePos());
    m_pMouseEvent->setScreenPos(ome->screenPos());
    m_pMouseEvent->setLastPos(ome->lastPos());
    m_pMouseEvent->setLastScenePos(ome->lastScenePos());
    m_pMouseEvent->setLastScreenPos(ome->lastScreenPos());
    m_pMouseEvent->setButtons(ome->buttons());
    m_pMouseEvent->setButton(ome->button());
    m_pMouseEvent->setModifiers(ome->modifiers());
}

MessageWidget* ToolBarState::getMessageAt(const QPointF& pos) {
    foreach (  MessageWidget* message, m_pUMLScene->getMessageList() ) {
        if (message->isVisible() && message->onWidget(pos)) {
            return message;
        }
    }

    return 0;
}

AssociationWidget* ToolBarState::getAssociationAt(const QPointF& pos) {

    foreach ( AssociationWidget* association, m_pUMLScene->getAssociationList() ) {
        if (association->onAssociation(pos)) {
            return association;
        }
    }

    return 0;
}

FloatingDashLineWidget* ToolBarState::getFloatingLineAt(const QPointF& pos) {
    FloatingDashLineWidget* floatingline = 0;

    foreach ( NewUMLRectWidget* widget, m_pUMLScene->getWidgetList() ) {
        if (widget->getBaseType() == Uml::wt_FloatingDashLine){
            if (dynamic_cast<FloatingDashLineWidget*>(widget)->contains(pos)) {
                floatingline = dynamic_cast<FloatingDashLineWidget*>(widget);
            }
        }
    }

    return floatingline;
}

#include "toolbarstate.moc"

