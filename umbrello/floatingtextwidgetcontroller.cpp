/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "floatingtextwidgetcontroller.h"
// qt/kde includes
#include <kdebug.h>
// app includes
#include "floatingtextwidget.h"
#include "messagewidget.h"
#include "objectwidget.h"

FloatingTextWidgetController::FloatingTextWidgetController(FloatingTextWidget *floatingTextWidget):
            UMLWidgetController(floatingTextWidget) {
    m_floatingTextWidget = floatingTextWidget;
    m_unconstrainedPositionX = 0;
    m_unconstrainedPositionY = 0;
    m_movementDirectionX = 0;
    m_movementDirectionY = 0;
}

FloatingTextWidgetController::~FloatingTextWidgetController() {
}

void FloatingTextWidgetController::saveWidgetValues(QMouseEvent *me) {
    UMLWidgetController::saveWidgetValues(me);

    m_unconstrainedPositionX = m_widget->getX();
    m_unconstrainedPositionY = m_widget->getY();
    m_movementDirectionX = 0;
    m_movementDirectionY = 0;
}

bool FloatingTextWidgetController::isInResizeArea(QMouseEvent* /*me*/) {
    return false;
}

void FloatingTextWidgetController::moveWidgetBy(int diffX, int diffY) {
    if (m_floatingTextWidget->m_Role == Uml::tr_Seq_Message_Self)
        return;

    if (m_floatingTextWidget->m_Role == Uml::tr_Seq_Message
                    && ((MessageWidget*)m_floatingTextWidget->m_pLink)->getSelected()) {
        return;
    }

    m_unconstrainedPositionX += diffX;
    m_unconstrainedPositionY += diffY;
    QPoint constrainedPosition = constrainPosition(diffX, diffY);

    int newX = constrainedPosition.x();
    int newY = constrainedPosition.y();

    if (!m_movementDirectionX) {
        if (m_unconstrainedPositionX != constrainedPosition.x()) {
            m_movementDirectionX = (diffX > 0)? 1: -1;
        }
    } else if ((m_movementDirectionX < 0 && m_unconstrainedPositionX > m_floatingTextWidget->getX()) ||
               (m_movementDirectionX > 0 && m_unconstrainedPositionX < m_floatingTextWidget->getX()) ) {
        newX = m_unconstrainedPositionX;
        m_movementDirectionX = 0;
    }

    if (!m_movementDirectionY) {
        if (m_unconstrainedPositionY != constrainedPosition.y()) {
            m_movementDirectionY = (diffY > 0)? 1: -1;
        }
    } else if ((m_movementDirectionY < 0 && m_unconstrainedPositionY > m_floatingTextWidget->getY()) ||
               (m_movementDirectionY > 0 && m_unconstrainedPositionY < m_floatingTextWidget->getY()) ) {
        newY = m_unconstrainedPositionY;
        m_movementDirectionY = 0;
    }

    m_floatingTextWidget->setX(newX);
    m_floatingTextWidget->setY(newY);

    if (m_floatingTextWidget->m_pLink) {
        m_floatingTextWidget->m_pLink->calculateNameTextSegment();
        if (m_floatingTextWidget->m_Role == Uml::tr_Seq_Message) {
            MessageWidget* messageWidget = (MessageWidget*)m_floatingTextWidget->m_pLink;
            messageWidget->setY(newY + m_floatingTextWidget->getHeight());

            //TODO This should be moved to somewhere in MessageWidget, refactor with messagewidgetcontroller.cpp:44
            if (messageWidget->getSequenceMessageType() == Uml::sequence_message_creation) {
                const int objWidgetHalfHeight = messageWidget->getWidget(Uml::B)->getHeight() / 2;
                messageWidget->getWidget(Uml::B)->UMLWidget::setY(messageWidget->getY() - objWidgetHalfHeight);
            }
        }
    }
}

void FloatingTextWidgetController::constrainMovementForAllWidgets(int &diffX, int &diffY) {
    QPoint constrainedPosition = constrainPosition(diffX, diffY);

    diffX = constrainedPosition.x() - m_floatingTextWidget->getX();
    diffY = constrainedPosition.y() - m_floatingTextWidget->getY();
}

QPoint FloatingTextWidgetController::constrainPosition(int diffX, int diffY) {
    int newX = m_floatingTextWidget->getX() + diffX;
    int newY = m_floatingTextWidget->getY() + diffY;

    if (m_floatingTextWidget->m_pLink) {
        m_floatingTextWidget->m_pLink->constrainTextPos(newX, newY,
                    m_floatingTextWidget->width(), m_floatingTextWidget->height(),
                    m_floatingTextWidget->m_Role);
    }

    return QPoint(newX, newY);
}

