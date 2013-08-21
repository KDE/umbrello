/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2013                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "messagewidgetcontroller.h"

// kde includes
#include <kcursor.h>

// app includes
#include "messagewidget.h"
#include "floatingtextwidget.h"
#include "objectwidget.h"
#include "listpopupmenu.h"

/**
 * Constructor for MessageWidgetController.
 *
 * @param messageWidget The message widget which uses the controller.
 */
MessageWidgetController::MessageWidgetController(MessageWidget* messageWidget)
  : UMLWidgetController(messageWidget)
{
    m_messageWidget = messageWidget;
}

/**
 * Destructor for MessageWidgetController.
 */
MessageWidgetController::~MessageWidgetController()
{
}

/**
 * Overridden from UMLWidgetController.
 * Saves the values of the widget needed for move/resize.
 * Calls parent method and then saves the value of m_unconstrainedPositionY
 *
 * @param me The QGraphicsSceneMouseEvent to get the offset from.
 */
//void MessageWidgetController::saveWidgetValues(QGraphicsSceneMouseEvent *me)
//{
//    UMLWidgetController::saveWidgetValues(me);

//    m_unconstrainedPositionY = m_widget->y();
//}

/**
 * Overridden from UMLWidgetController.
 * Returns the cursor to be shown when resizing the widget.
 * The cursor shown is KCursor::sizeVerCursor().
 *
 * @return The cursor to be shown when resizing the widget.
 */
QCursor MessageWidgetController::getResizeCursor()
{
    return Qt::SizeVerCursor;
}

/**
 * Overridden from UMLWidgetController.
 * Executes the action for double click in the widget.
 * Shows the dialog to select the operation of the message.
 *
 * @param me The QGraphicsSceneMouseEvent which triggered the double click event.
 */
//void MessageWidgetController::doMouseDoubleClick(QGraphicsSceneMouseEvent* me)
//{
//    Q_UNUSED(me);
//    if (m_messageWidget->m_pFText != NULL) {
//        QAction* action = m_messageWidget->m_pMenu->getAction(ListPopupMenu::mt_Select_Operation);
//        m_messageWidget->m_pFText->slotMenuSelection(action);
//    }
//}

