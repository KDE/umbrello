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
#include "uml.h"
#include "umlview.h"
#include "umlwidget.h"

ToolBarState::~ToolBarState() {
    delete m_pMouseEvent;
}

void ToolBarState::init() {
    m_pUMLView->viewport()->setMouseTracking(false);
    m_pMouseEvent = 0;
    m_currentWidget = 0;
    m_currentAssociation = 0;

    connect(m_pUMLView, SIGNAL(sigAssociationRemoved(AssociationWidget*)),
            this, SLOT(slotAssociationRemoved(AssociationWidget*)));
    connect(m_pUMLView, SIGNAL(sigWidgetRemoved(UMLWidget*)),
            this, SLOT(slotWidgetRemoved(UMLWidget*)));
}

void ToolBarState::cleanBeforeChange() {
    disconnect(m_pUMLView, SIGNAL(sigAssociationRemoved(AssociationWidget*)),
               this, SLOT(slotAssociationRemoved(AssociationWidget*)));
    disconnect(m_pUMLView, SIGNAL(sigWidgetRemoved(UMLWidget*)),
               this, SLOT(slotWidgetRemoved(UMLWidget*)));
}

void ToolBarState::mousePress(QMouseEvent* ome) {
    setMouseEvent(ome, QEvent::MouseButtonPress);

    m_pUMLView->viewport()->setMouseTracking(true);

    //TODO Doesn't another way of emiting the signal exist? A method only for
    //that seems a bit dirty.
    m_pUMLView->emitRemovePopupMenu();

    // TODO: Check who needs this.
    m_pUMLView->setPos(m_pMouseEvent->pos());

    //TODO check why
    m_pUMLView->setPaste(false);

    setCurrentElement();

    if (getCurrentWidget()) {
        mousePressWidget();
    } else if (getCurrentAssociation()) {
        mousePressAssociation();
    } else {
        mousePressEmpty();
    }
}

void ToolBarState::mouseRelease(QMouseEvent* ome) {
    setMouseEvent(ome, QEvent::MouseButtonRelease);

    // Set the position of the mouse
    // TODO, should only be available in this state?
    m_pUMLView->setPos(m_pMouseEvent->pos());

    m_pUMLView->viewport()->setMouseTracking(false);

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

void ToolBarState::mouseDoubleClick(QMouseEvent* ome) {
    setMouseEvent(ome, QEvent::MouseButtonDblClick);

    UMLWidget* currentWidget = m_pUMLView->getWidgetAt(m_pMouseEvent->pos());
    AssociationWidget* currentAssociation = getAssociationAt(m_pMouseEvent->pos());
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

void ToolBarState::mouseMove(QMouseEvent* ome) {
    setMouseEvent(ome, QEvent::MouseMove);

    if (getCurrentWidget()) {
        mouseMoveWidget();
    } else if (getCurrentAssociation()) {
        mouseMoveAssociation();
    } else {
        mouseMoveEmpty();
    }

    //Scrolls the view
    int vx = ome->x();
    int vy = ome->y();
    int contsX = m_pUMLView->contentsX();
    int contsY = m_pUMLView->contentsY();
    int visw = m_pUMLView->visibleWidth();
    int vish = m_pUMLView->visibleHeight();
    int dtr = visw - (vx-contsX);
    int dtb = vish - (vy-contsY);
    int dtt =  (vy-contsY);
    int dtl =  (vx-contsX);
    if (dtr < 30) m_pUMLView->scrollBy(30-dtr,0);
    if (dtb < 30) m_pUMLView->scrollBy(0,30-dtb);
    if (dtl < 30) m_pUMLView->scrollBy(-(30-dtl),0);
    if (dtt < 30) m_pUMLView->scrollBy(0,-(30-dtt));
}

void ToolBarState::slotAssociationRemoved(AssociationWidget* association) {
    if (association == getCurrentAssociation()) {
        setCurrentAssociation(0);
    }
}

void ToolBarState::slotWidgetRemoved(UMLWidget* widget) {
    if (widget == getCurrentWidget()) {
        setCurrentWidget(0);
    }
}

ToolBarState::ToolBarState(UMLView *umlView) : QObject(umlView), m_pUMLView(umlView) {
    m_pMouseEvent = NULL;
    init();
}

void ToolBarState::setCurrentElement() {
    // Check associations.
    AssociationWidget* association = getAssociationAt(m_pMouseEvent->pos());
    if (association) {
        setCurrentAssociation(association);
        return;
    }

    // Check messages.
    //TODO check why message widgets are treated different
    MessageWidget* message = getMessageAt(m_pMouseEvent->pos());
    if (message) {
        setCurrentWidget(message);
        return;
    }

    // Check widgets.
    UMLWidget *widget = m_pUMLView->getWidgetAt(m_pMouseEvent->pos());
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
    m_pUMLView->clearSelected();
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
    m_pUMLView->clearSelected();
}

void ToolBarState::mouseMoveAssociation() {
}

void ToolBarState::mouseMoveWidget() {
}

void ToolBarState::mouseMoveEmpty() {
}

void ToolBarState::changeTool() {
    if (m_pMouseEvent->state() == Qt::RightButton) {
        UMLApp::app()->getWorkToolBar()->setDefaultTool();
    }
}

void ToolBarState::setMouseEvent(QMouseEvent* ome, const QEvent::Type &type) {
    if (m_pMouseEvent) delete m_pMouseEvent;

    m_pMouseEvent = new QMouseEvent(type, m_pUMLView->inverseWorldMatrix().map(ome->pos()),
                                    ome->button(),ome->state());
}

MessageWidget* ToolBarState::getMessageAt(const QPoint& pos) {
    MessageWidget* message = 0;
    for (MessageWidgetListIt it(m_pUMLView->getMessageList());
                                (message = it.current()) != 0; ++it) {
        if (message->isVisible() && message->onWidget(pos)) {
            return message;
        }
    }

    return message;
}

AssociationWidget* ToolBarState::getAssociationAt(const QPoint& pos) {
    AssociationWidget* association = 0;
    for (AssociationWidgetListIt it(m_pUMLView->getAssociationList());
                                (association = it.current()) != 0; ++it) {
        if (association->onAssociation(pos)) {
            return association;
        }
    }

    return association;
}

#include "toolbarstate.moc"
