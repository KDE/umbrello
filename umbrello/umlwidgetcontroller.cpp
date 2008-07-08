/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umlwidgetcontroller.h"

// qt includes
#include <QtCore/QEvent>
#include <QtCore/QPoint>

// kde includes
#include <kcursor.h>
#include <kdebug.h>
#include <klocale.h>

// app includes
#include "umlwidget.h"
#include "umlwidgetlist.h"
#include "umlnamespace.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "umlobject.h"
#include "listpopupmenu.h"
#include "classifierwidget.h"
#include "associationwidget.h"
#include "messagewidget.h"
#include "cmds.h"
#include "umlscene.h"

using namespace Uml;

UMLWidgetController::UMLWidgetController(NewUMLRectWidget *widget)
{
    m_widget = widget;

    m_oldX = m_oldY = 0;
    m_pressOffsetX = m_pressOffsetY = 0;
    m_prevX = m_prevY = 0;
    m_oldW = m_oldH = 0;

    m_minSelectedX = m_minSelectedY = m_maxSelectedX = m_maxSelectedY = 0;

    m_shiftPressed = false;
    m_leftButtonDown = m_middleButtonDown = m_rightButtonDown = false;
    m_inMoveArea = m_inResizeArea = 0;
    m_wasSelected = m_moved = m_resized = 0;
}

UMLWidgetController::~UMLWidgetController()
{
}

void UMLWidgetController::mousePressEvent(QGraphicsSceneMouseEvent *me)
{
    // If there is a button pressed already ignore other press events
    if (m_leftButtonDown || m_middleButtonDown || m_rightButtonDown) {
        return;
    }

    if (me->button() == Qt::LeftButton) {
        m_leftButtonDown = true;
        //saveWidgetValues(me);
    } else if (me->button() == Qt::RightButton) {
        m_rightButtonDown = true;
    } else {
        m_middleButtonDown = true;
        return;
    }

    //There is no harm in saving all the values of the widget even when
    //they aren't going to be used
    saveWidgetValues(me);

    m_oldStatusBarMsg = UMLApp::app()->getStatusBarMsg();

    if (me->modifiers() == Qt::ShiftModifier || me->modifiers() == Qt::ControlModifier) {
        m_shiftPressed = true;

        if (me->button() == Qt::LeftButton) {
            m_inMoveArea = true;
        }

        if (!m_widget->isSelected()) {
            selectMultiple(me);
        } else if (!m_rightButtonDown) {
            m_wasSelected = false;
        }
        return;
    }

    m_shiftPressed = false;

    int count = m_widget->umlScene()->getSelectCount(true);
    if (me->button() == Qt::LeftButton) {
        if (m_widget->isSelected() && count > 1) {
            //Single selection is made in release event if the widget wasn't moved
            m_inMoveArea = true;
            lastUpdate.start();
            return;
        }

        if (isInResizeArea(me)) {
            m_inResizeArea = true;
        } else {
            m_inMoveArea = true;
        }
    }

    //If widget wasn't selected, or it was selected but with other widgets also selected
    if (!m_widget->isSelected() || count > 1) {
        selectSingle(me);
    } else if (!m_rightButtonDown) {
        m_wasSelected = false;
    }
}

void UMLWidgetController::mouseMoveEvent(QGraphicsSceneMouseEvent* me)
{
    if (!m_leftButtonDown)
        return;

    if (m_inResizeArea) {
        resize(me);
        return;
    }

    if (!m_moved) {
        UMLApp::app()->getDocument()->writeToStatusBar(i18n("Hold shift or ctrl to move in X axis. Hold shift and control to move in Y axis. Right button click to cancel move."));

        m_moved = true;
        //Maybe needed by AssociationWidget
        m_widget->m_bStartMove = true;

        setSelectionBounds();
    }

    QPointF position = getPosition(me);
    qreal diffX = position.x() - m_widget->getX();
    qreal diffY = position.y() - m_widget->getY();

    if ((me->modifiers() & Qt::ShiftModifier) && (me->modifiers() & Qt::ControlModifier)) {
        //Move in Y axis
        diffX = 0;
    } else if ((me->modifiers() & Qt::ShiftModifier) || (me->modifiers() & Qt::ControlModifier)) {
        //Move in X axis
        diffY = 0;
    }

    /* Commands
        UMLDoc* doc = UMLApp::app()->getDocument();
        doc->executeCommand(new CmdMoveWidget(this,diffX,diffY));*/

    // moveWidget(diffX,diffY);

    // uDebug() << "before constrainMovementForAllWidgets:"
    //     << " diffX=" << diffX << ", diffY=" << diffY << endl;
    constrainMovementForAllWidgets(diffX, diffY);
    // uDebug() << "after constrainMovementForAllWidgets:"
    //     << " diffX=" << diffX << ", diffY=" << diffY << endl;

    //Nothing to move
    if (diffX == 0 && diffY == 0) {
        return;
    }

    bool update = false;
    if (lastUpdate.elapsed() > 25) {
        update = true;
        lastUpdate.restart();

        m_widget->adjustUnselectedAssocs(m_widget->getX(), m_widget->getY());
    }

    foreach(NewUMLRectWidget* widget , m_selectedWidgetsList) {

        //UMLDoc* m_doc = UMLApp::app()->getDocument();
        //CmdMoveWidgetBy* cmd = new CmdMoveWidgetBy(widget,diffX,diffY);
        //m_doc->executeCommand(cmd);
        //m_doc->executeCommand(new CmdMoveWidgetBy(widget,diffX,diffY));
        widget->getWidgetController()->moveWidgetBy(diffX, diffY);
    }
    // uDebug();

    // Move any selected associations.

    foreach(AssociationWidget* aw, m_widget->umlScene()->getSelectedAssocs()) {
        if (aw->getSelected()) {
            aw->moveEntireAssoc(diffX, diffY);
        }
    }

    m_widget->umlScene()->resizeCanvasToItems();
    updateSelectionBounds(diffX, diffY);

}

void UMLWidgetController::widgetMoved()
{
    //reverseOldNewValues();
    m_moved = false;

    //Ensure associations are updated (the timer could prevent the
    //adjustment in the last move event before the release)

    foreach(NewUMLRectWidget* widget , m_selectedWidgetsList) {

        widget->adjustAssocs(widget->getX(), widget->getY());
    }

    m_widget->m_bStartMove = false;
}


void UMLWidgetController::mouseReleaseEvent(QGraphicsSceneMouseEvent *me)
{
    if (me->button() != Qt::LeftButton && me->button() != Qt::RightButton) {
        if (m_middleButtonDown) {
            m_middleButtonDown = false;
            resetSelection();
        }
    } else if (me->button() == Qt::LeftButton) {
        if (m_leftButtonDown) {
            m_leftButtonDown = false;

            if (!m_moved && !m_resized) {
                if (!m_shiftPressed && (m_widget->umlScene()->getSelectCount(true) > 1)) {
                    selectSingle(me);
                } else if (!m_wasSelected) {
                    deselect(me);
                }
            } else {
                if (m_moved) {

                    /* Commands */
                    UMLApp::app()->executeCommand(new CmdMoveWidget(this));

                } else {
                    UMLApp::app()->executeCommand(new CmdResizeWidget(this));
                    m_resized = false;
                }

                if ((m_inMoveArea && wasPositionChanged()) ||
                        (m_inResizeArea && wasSizeChanged())) {
                    m_widget->umlDoc()->setModified(true);
                }

                UMLApp::app()->getDocument()->writeToStatusBar(m_oldStatusBarMsg);
            }

            if (m_inResizeArea) {
                m_inResizeArea = false;
                m_widget->setCursor(Qt::ArrowCursor);
            } else {
                m_inMoveArea = false;
            }
        }
    } else if (me->button() == Qt::RightButton) {
        if (m_rightButtonDown) {
            m_rightButtonDown = false;
            showPopupMenu(me);
        } else if (m_leftButtonDown) {
            //Cancel move/edit
            // [PORT] The below code needs a proper port.
#if 0
            QGraphicsSceneMouseEvent move(QGraphicsSceneMouseEvent::MouseMove,
                             QPointF(m_oldX + m_pressOffsetX, m_oldY + m_pressOffsetY),
                             Qt::LeftButton, Qt::NoButton);
            mouseMoveEvent(&move);
            QGraphicsSceneMouseEvent release(QGraphicsSceneMouseEvent::MouseButtonRelease,
                                QPointF(m_oldX + m_pressOffsetX, m_oldY + m_pressOffsetY),
                                Qt::LeftButton, Qt::NoButton);
            mouseReleaseEvent(&release);
#endif
        }
    }

    //TODO Copied from old code. Does it really work as intended?
    NewUMLRectWidget *bkgnd = m_widget->umlScene()->getWidgetAt(me->pos());
    if (bkgnd) {
        //uDebug() << "setting Z to " << bkgnd->getZ() + 1 << endl;
        m_widget->setZ(bkgnd->getZ() + 1);
    } else {
        m_widget->setZ(0);
    }
}

NewUMLRectWidget* UMLWidgetController::getWidget()
{
    return m_widget;
}

void UMLWidgetController::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *me)
{
    if (me->button() != Qt::LeftButton) {
        return;
    }

    selectSingle(me);

    doMouseDoubleClick(me);
}

bool UMLWidgetController::isInResizeArea(QGraphicsSceneMouseEvent *me)
{
    const qreal m = 10;

    //note The following directly represents the local position for the widget
    qreal x = me->pos().x(), y = me->pos().y();
    if (m_widget->isResizable() &&
            x >= (m_widget->getWidth() - m) &&
            y >= (m_widget->getHeight() - m)) {
        m_widget->setCursor(getResizeCursor());
        return true;
    } else {
        m_widget->setCursor(Qt::ArrowCursor);
        return false;
    }
}

QCursor UMLWidgetController::getResizeCursor()
{
    return Qt::SizeFDiagCursor;
}

void UMLWidgetController::resizeWidget(qreal newW, qreal newH)
{
    m_widget->setSize(newW, newH);
}

void UMLWidgetController::moveWidgetBy(qreal diffX, qreal diffY)
{
    m_widget->setX(m_widget->getX() + diffX);
    m_widget->setY(m_widget->getY() + diffY);
}

void UMLWidgetController::constrainMovementForAllWidgets(qreal &/*diffX*/, qreal &/*diffY*/)
{
}

void UMLWidgetController::doMouseDoubleClick(QGraphicsSceneMouseEvent *)
{
    if (!m_widget || !m_widget->m_pMenu)
        return;
    QAction* action = m_widget->m_pMenu->getAction(ListPopupMenu::mt_Properties);
    m_widget->slotMenuSelection(action);
}

void UMLWidgetController::resetSelection()
{
    m_widget->umlScene()->clearSelected();
    m_widget->umlScene()->resetToolbar();
    // The widget is already cleared from selection.
    m_wasSelected = false;
}

void UMLWidgetController::selectSingle(QGraphicsSceneMouseEvent *me)
{
    m_widget->umlScene()->clearSelected();

    //Adds the widget to the selected widgets list, but as it has been cleared
    //only the current widget is selected
    selectMultiple(me);
}

void UMLWidgetController::selectMultiple(QGraphicsSceneMouseEvent *me)
{
    m_widget->umlScene()->setSelected(m_widget, me);
    //scene->setSelected also sets the widgets local status to selected.
    m_wasSelected = true;
}

void UMLWidgetController::deselect(QGraphicsSceneMouseEvent *me)
{
    // [PORT] Strange, only flag is unselected while it is still in
    // selected list. It is hard to reflect this behavior in
    // QGraphicsScene. Investigate this issue properly.
    m_widget->setSelected(false);
#if 0
    m_widget->m_bSelected = false;
    m_widget->setSelected(m_widget->isSelected());
    m_widget->umlScene()->setSelected(m_widget, me);
    //m_wasSelected is false implicitly, no need to set it again
#endif
}

void UMLWidgetController::saveWidgetValues(QGraphicsSceneMouseEvent *me)
{
    m_pressOffsetX = me->scenePos().x() - m_widget->getX();
    m_pressOffsetY = me->scenePos().y() - m_widget->getY();

    m_prevX = m_oldX = m_widget->getX();
    m_prevY = m_oldY = m_widget->getY();

    m_oldW = m_widget->getWidth();
    m_oldH = m_widget->getHeight();
}

void UMLWidgetController::insertSaveValues(qreal _oldX, qreal _oldY, qreal X, qreal Y)
{
    m_widget->setX(X);
    m_widget->setY(Y);
    m_oldX = _oldX;
    m_oldY = _oldY;
}

qreal UMLWidgetController::getOldX()
{
    return m_oldX;
}

qreal UMLWidgetController::getOldY()
{
    return m_oldY;
}

qreal UMLWidgetController::getOldW()
{
    return m_oldW;
}

qreal UMLWidgetController::getOldH()
{
    return m_oldH;
}

void UMLWidgetController::setSelectionBounds()
{
    if (m_widget->umlScene()->getSelectCount() > 0) {
        m_selectedWidgetsList.clear();
        m_widget->umlScene()->getSelectedWidgets(m_selectedWidgetsList, false);

        updateSelectionBounds(1, 1);
    }
}

//TODO optimize it
void UMLWidgetController::updateSelectionBounds(qreal diffX, qreal diffY)
{
    if (diffX != 0) {
        m_minSelectedX = getSmallestX(m_selectedWidgetsList);
        m_maxSelectedX = getBiggestX(m_selectedWidgetsList);
    }
    if (diffY != 0) {
        m_minSelectedY = getSmallestY(m_selectedWidgetsList);
        m_maxSelectedY = getBiggestY(m_selectedWidgetsList);
    }
}

void UMLWidgetController::resize(QGraphicsSceneMouseEvent *me)
{
    UMLApp::app()->getDocument()->writeToStatusBar(i18n("Hold shift or ctrl to move in X axis. Hold shift and control to move in Y axis. Right button click to cancel resize."));

    m_resized = true;

    qreal newW = m_oldW + me->scenePos().x() - m_widget->getX() - m_pressOffsetX;
    qreal newH = m_oldH + me->scenePos().y() - m_widget->getY() - m_pressOffsetY;

    if ((me->modifiers() & Qt::ShiftModifier) && (me->modifiers() & Qt::ControlModifier)) {
        //Move in Y axis
        newW = m_oldW;
    } else if ((me->modifiers() & Qt::ShiftModifier) || (me->modifiers() & Qt::ControlModifier)) {
        //Move in X axis
        newH = m_oldH;
    }

    m_widget->constrain(newW, newH);
    resizeWidget(newW, newH);
    m_widget->adjustAssocs(m_widget->getX(), m_widget->getY());

    m_widget->umlScene()->resizeCanvasToItems();
}

//TODO refactor with AlignToolbar method.
qreal UMLWidgetController::getSmallestX(const UMLWidgetList &widgetList)
{
    qreal smallestX = 0;
    int i = 1;
    foreach(NewUMLRectWidget* widget , widgetList) {
        if (i == 1) {
            if (widget == NULL)
                break;
            smallestX = widget->getX();
        } else {
            if (smallestX > widget->getX())
                smallestX = widget->getX();
        }
        i++;
    }

    return smallestX;
}

//TODO refactor with AlignToolbar method.
qreal UMLWidgetController::getSmallestY(const UMLWidgetList &widgetList)
{
    qreal smallestY = 0;
    int i = 1;
    foreach(NewUMLRectWidget* widget , widgetList) {
        if (i == 1) {
            if (widget == NULL)
                break;
            smallestY = widget->getY();
        } else {
            if (smallestY > widget->getY())
                smallestY = widget->getY();

        }
        i++;
    }

    return smallestY;
}

//TODO refactor with AlignToolbar method.
qreal UMLWidgetController::getBiggestX(const UMLWidgetList &widgetList)
{
    qreal biggestX = 0;

    int i = 1;
    foreach(NewUMLRectWidget* widget , widgetList) {
        if (i == 1) {
            if (widget == NULL)
                break;
            biggestX = widget->getX();
            biggestX += widget->getWidth();
        } else {
            if (biggestX < widget->getX() + widget->getWidth())
                biggestX = widget->getX() + widget->getWidth();
        }
        i++;
    }

    return biggestX;
}

//TODO refactor with AlignToolbar method.
qreal UMLWidgetController::getBiggestY(const UMLWidgetList &widgetList)
{
    qreal biggestY = 0;
    int i = 1;
    foreach(NewUMLRectWidget* widget , widgetList) {
        if (i == 1) {
            if (widget == NULL)
                break;
            biggestY = widget->getY();
            biggestY += widget->getHeight();
        } else {
            if (biggestY < widget->getY() + widget->getHeight())
                biggestY = widget->getY() + widget->getHeight();
        }
        i++;
    }

    return biggestY;
}

QPointF UMLWidgetController::getPosition(QGraphicsSceneMouseEvent* me)
{
    /*
    uDebug() << "me->x=" << me->x()
        << " m_widget->getX=" << m_widget->getX() << ", m_oldX=" << m_oldX
        << ", m_pressOffsetX=" << m_pressOffsetX << endl;
    uDebug() << "me->y=" << me->y()
        << " m_widget->getY=" << m_widget->getY() << ", m_oldY=" << m_oldY
        << ", m_pressOffsetY=" << m_pressOffsetY << endl;
     */

    // [PORT] Verify this x and y values.
    qreal x = me->scenePos().x(), y = me->scenePos().y();

    qreal newX = x + m_widget->getX() - m_prevX - m_pressOffsetX;
    qreal newY = y + m_widget->getY() - m_prevY - m_pressOffsetY;

    qreal maxX = m_widget->umlScene()->width();
    qreal maxY = m_widget->umlScene()->height();

    m_prevX = newX;
    m_prevY = newY;

    if (newX + (m_minSelectedX - m_widget->getX()) < 0) {
        //uDebug() << "got into cond.1";
        newX = m_widget->getX() - m_minSelectedX;
    }
    if (newY + (m_minSelectedY - m_widget->getY()) < 0) {
        //uDebug() << "got into cond.2";
        newY = m_widget->getY() - m_minSelectedY;
    }
    if (newX + (m_maxSelectedX - m_widget->getX()) > maxX) {
        //uDebug() << "got into cond.3";
        newX = maxX - (m_maxSelectedX - m_widget->getX());
    }
    if (newY + (m_maxSelectedY - m_widget->getY()) > maxY) {
        //uDebug() << "got into cond.4";
        newY = maxY - (m_maxSelectedY - m_widget->getY());
    }
    return QPointF(newX, newY);
}

QPointF UMLWidgetController::getPositionDifference(QGraphicsSceneMouseEvent* me)
{
    QPointF newPoint = getPosition(me);
    const qreal diffX = newPoint.x() - m_widget->getX();
    const qreal diffY = newPoint.y() - m_widget->getY();
    return QPointF(diffX, diffY);
}

void UMLWidgetController::showPopupMenu(QGraphicsSceneMouseEvent *me)
{
    //TODO why this condition?
    if (m_widget->m_pMenu) {
        return;
    }
    ListPopupMenu* menu = m_widget->setupPopupMenu();
    menu->popup(me->screenPos());
}

bool UMLWidgetController::wasSizeChanged()
{
    return m_oldW != m_widget->getWidth() || m_oldH != m_widget->getHeight();
}

bool UMLWidgetController::wasPositionChanged()
{
    return m_oldX != m_widget->getX() || m_oldY != m_widget->getY();
}
