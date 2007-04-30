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

// app includes
#include "objectwidget.h"
#include "listpopupmenu.h"


ObjectWidgetController::ObjectWidgetController(ObjectWidget* objectWidget):
            UMLWidgetController(objectWidget) {
}

ObjectWidgetController::~ObjectWidgetController() {
}

QCursor ObjectWidgetController::getResizeCursor() {
    return KCursor::sizeHorCursor();
}

void ObjectWidgetController::resizeWidget(int newW, int newH) {
    m_widget->setSize(newW, m_widget->getHeight());
}

void ObjectWidgetController::moveWidgetBy(int diffX, int diffY) {
    m_widget->setX(m_widget->getX() + diffX);
}

void ObjectWidgetController::constrainMovementForAllWidgets(int &diffX, int &diffY) {
    diffY = 0;
}
