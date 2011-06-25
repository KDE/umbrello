/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef OBJECTWIDGETCONTROLLER_H
#define OBJECTWIDGETCONTROLLER_H

#include "umlwidgetcontroller.h"

class ObjectWidget;

/**
 * Controller for ObjectWidget.
 *
 * When moving an ObjectWidget, it is only moved along X axis. Y axis movement
 * is always ignored.
 * So, if the ObjectWidget is being moved as part of a selection and that
 * selection is moved in X and/or Y axis, the ObjectWidget will only move in X axis.
 * Also, when constraining the move of the selection because the receiver of
 * mouse move events is an ObjectWidget, all the widgets are moved only in X axis.
 *
 * Only horizontal resize is allowed for ObjectWidget. Cursor is set to reflect this.
 *
 * @author Umbrello UML Modeller Authors <uml-devel@lists.sourceforge.net>
 */
class ObjectWidgetController : public UMLWidgetController
{
public:

    ObjectWidgetController(ObjectWidget *objectWidget);
    virtual ~ObjectWidgetController();

protected:

    virtual QCursor getResizeCursor();

    virtual void resizeWidget(int newW, int newH);

    virtual void mousePressEvent(QMouseEvent *me);
    virtual void mouseMoveEvent(QMouseEvent* me);

    virtual void moveWidgetBy(int diffX, int diffY);

    void moveDestructionBy (int diffY);

    virtual void constrainMovementForAllWidgets(int &diffX, int &diffY);

    bool m_isOnDestructionBox;  ///< true when a click occurred on the destruction box
};

#endif
