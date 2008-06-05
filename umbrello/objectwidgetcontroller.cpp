/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "objectwidgetcontroller.h"

// kde includes
#include <kcursor.h>
#include <kdebug.h>
#include <klocale.h>

// app includes
#include "objectwidget.h"
#include "seqlinewidget.h"
#include "umlnamespace.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "umlwidget.h"
#include "umlwidgetlist.h"
#include "umlobject.h"
#include "classifierwidget.h"
#include "associationwidget.h"
#include "messagewidget.h"


ObjectWidgetController::ObjectWidgetController(ObjectWidget* objectWidget):
            UMLWidgetController(objectWidget) {
}

ObjectWidgetController::~ObjectWidgetController() {
}

QCursor ObjectWidgetController::getResizeCursor() {
    return Qt::SizeHorCursor;
}

void ObjectWidgetController::resizeWidget(qreal newW, qreal /*newH*/) {
    m_widget->setSize(newW, m_widget->getHeight());
}

void ObjectWidgetController::mousePressEvent(QGraphicsSceneMouseEvent *me) {
    UMLWidgetController::mousePressEvent(me);
     isOnDestructionBox = false;
    SeqLineWidget * pLine = dynamic_cast<ObjectWidget*>(m_widget)->getSeqLine();

    if (pLine->onDestructionBox(me->pos())) {
        isOnDestructionBox = true;
        m_oldX = dynamic_cast<ObjectWidget*>(m_widget)->getX() + dynamic_cast<ObjectWidget*>(m_widget)->getWidth() / 2;
        m_oldY = dynamic_cast<ObjectWidget*>(m_widget)->getEndLineY() - 10;
    }

}

void ObjectWidgetController::mouseMoveEvent(QGraphicsSceneMouseEvent* me) {
    if (!m_leftButtonDown)
        return;

    if (m_inResizeArea) {
        resize(me);
        return;
    }

    // [PORT]
    qreal diffY = me->scenePos().y() - m_oldY;

    if (isOnDestructionBox) {
        moveDestructionBy (diffY);
    }

    else
        UMLWidgetController::mouseMoveEvent(me);

}

void ObjectWidgetController::moveWidgetBy(qreal diffX, qreal /*diffY*/) {
    m_widget->setX(m_widget->getX() + diffX);
}

void ObjectWidgetController::moveDestructionBy(qreal diffY) {
    // endLine = length of the life line + diffY - 10 to center on the destruction box
    qreal endLine = dynamic_cast<ObjectWidget *>(m_widget)->getEndLineY() + diffY - 10;
    SeqLineWidget * pLine = dynamic_cast<ObjectWidget *>(m_widget)->getSeqLine();
    pLine->setEndOfLine(endLine);
    m_oldY = endLine;
}

void ObjectWidgetController::constrainMovementForAllWidgets(qreal& /*diffX*/, qreal& diffY) {
    diffY = 0;
}
