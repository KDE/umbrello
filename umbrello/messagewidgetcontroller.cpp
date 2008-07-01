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
#include "messagewidgetcontroller.h"

// kde includes
#include <kcursor.h>
#include <kdebug.h>

// app includes
#include "messagewidget.h"
#include "floatingtextwidget.h"
#include "objectwidget.h"
#include "listpopupmenu.h"

MessageWidgetController::MessageWidgetController(MessageWidget* messageWidget):
            UMLWidgetController(messageWidget) {
    m_messageWidget = messageWidget;
    m_unconstrainedPositionY = 0;
}

MessageWidgetController::~MessageWidgetController() {
}

void MessageWidgetController::saveWidgetValues(QGraphicsSceneMouseEvent *me) {
    UMLWidgetController::saveWidgetValues(me);

    m_unconstrainedPositionY = m_widget->getY();
}

QCursor MessageWidgetController::getResizeCursor() {
    return Qt::SizeVerCursor;
}

void MessageWidgetController::resizeWidget(qreal newW, qreal newH) {
    if (m_messageWidget->getSequenceMessageType() == Uml::sequence_message_creation)
        m_messageWidget->setSize(m_messageWidget->getWidth(), newH);
    else {
        qreal x1 = m_messageWidget->m_pOw[Uml::A]->getX();
        qreal x2 = m_messageWidget->getxclicked();
        qreal diffX = 0;
        if (x1 < x2) {
            diffX = x2 + (newW - m_messageWidget->getWidth());
        }
        else {
            diffX = x2 - (newW - m_messageWidget->getWidth());
        }
        if (diffX <= 0 )
            diffX = 10;
        m_messageWidget->setxclicked (diffX);
        m_messageWidget->setSize(newW, newH);
        m_messageWidget->calculateWidget();

    }
    emit m_messageWidget->sigMessageMoved();
}

void MessageWidgetController::moveWidgetBy(qreal /*diffX*/, qreal diffY) {
    m_unconstrainedPositionY += diffY;
    qreal newY = constrainPositionY(diffY);

    if (m_unconstrainedPositionY != newY) {
        if (m_unconstrainedPositionY > m_messageWidget->getY()) {
            newY = m_unconstrainedPositionY;
        } else {
            return;
        }
    }

    m_messageWidget->setY(newY);

    if (m_messageWidget->m_sequenceMessageType == Uml::sequence_message_creation) {
        const qreal objWidgetHalfHeight = m_messageWidget->m_pOw[Uml::B]->getHeight() / 2;
        m_messageWidget->m_pOw[Uml::B]->NewUMLRectWidget::setY(newY - objWidgetHalfHeight);
    }

    m_messageWidget->moveEvent(0);
}

void MessageWidgetController::constrainMovementForAllWidgets(qreal &diffX, qreal &diffY) {
    diffX = 0;
    diffY = constrainPositionY(diffY) - m_widget->getY();
}

void MessageWidgetController::doMouseDoubleClick(QGraphicsSceneMouseEvent* /*me*/) {
    if (m_messageWidget->m_pFText != NULL) {
        QAction* action = m_messageWidget->m_pMenu->getAction(ListPopupMenu::mt_Select_Operation);
        m_messageWidget->m_pFText->slotMenuSelection(action);
    }
}

qreal MessageWidgetController::constrainPositionY(qreal diffY) {
    qreal newY = m_widget->getY() + diffY;

    qreal minY = m_messageWidget->getMinY();
    if (m_messageWidget->m_pFText && !m_messageWidget->m_pFText->getDisplayText().isEmpty()) {
        minY += m_messageWidget->m_pFText->getHeight();
    }

    if (newY < minY) {
        newY = minY;
    }

    return newY;
}

