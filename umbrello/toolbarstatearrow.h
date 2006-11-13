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

#ifndef TOOLBARSTATEARROW_H
#define TOOLBARSTATEARROW_H


#include "toolbarstate.h"

#include "worktoolbar.h"

class QMouseEvent;
class UMLView;

class QCanvasLine;

/**
 * Arrow tool for select, move and resize widgets and associations.
 * Arrow tool delegates the event handling in the widgets and associations. When
 * no widget nor association is being used,the arrow tool acts as a selecting
 * tool that selects all the elements in the rectangle created when dragging the
 * mouse.
 *
 * This is the default tool.
 */
class ToolBarStateArrow : public ToolBarState {
    Q_OBJECT
public:

    /**
     * Creates a new ToolBarStateArrow.
     *
     * @param umlView The UMLView to use.
     */
    ToolBarStateArrow(UMLView *umlView);

    /**
     * Destroys this ToolBarStateArrow.
     */
    virtual ~ToolBarStateArrow();

    /**
     * Goes back to the inital state.
     */
    virtual void init();

protected:

    /**
     * Called when the press event happened on an association.
     * Delivers the event to the association.
     */
    virtual void mousePressAssociation();

    /**
     * Called when the press event happened on a widget.
     * Delivers the event to the widget.
     */
    virtual void mousePressWidget();

    /**
     * Called when the press event happened on an empty space.
     * Calls base method and, if left button was pressed, prepares the selection
     * rectangle.
     */
    virtual void mousePressEmpty();

    /**
     * Called when the release event happened on an association.
     * Delivers the event to the association.
     */
    virtual void mouseReleaseAssociation();

    /**
     * Called when the release event happened on a widget.
     * Delivers the event to the widget.
     */
    virtual void mouseReleaseWidget();

    /**
     * Called when the release event happened on an empty space.
     * If selection rectangle is active, it is cleared. Else, if the right
     * button was released, it shows the pop up menu for the diagram.
     */
    virtual void mouseReleaseEmpty();

    /**
     * Called when the double click event happened on an association.
     * Delivers the event to the association.
     */
    virtual void mouseDoubleClickAssociation();

    /**
     * Called when the double click event happened on a widget.
     * Delivers the event to the widget.
     */
    virtual void mouseDoubleClickWidget();

    /**
     * Called when the move event happened when an association is
     * currently available.
     * Delivers the event to the association.
     */
    virtual void mouseMoveAssociation();

    /**
     * Called when the move event happened when a widget is
     * currently available.
     * Delivers the event to the widget.
     */
    virtual void mouseMoveWidget();

    /**
     * Called when the move event happened when no association nor
     * widget are currently available.
     * Updates the selection rectangle to the new position and selectes all the
     * widgets in the rectangle.
     *
     * @todo Fix selection
     */
    virtual void mouseMoveEmpty();

    /**
     * Sets the widget currently in use.
     * It ensures that the widget is only set if there is no other widget set
     * already.
     * It avoids things like moving a big widget over a little one, clicking
     * right button to cancel the movement and the little widget getting the
     * event, thus not cancelling the movement in the big widget.
     */
    virtual void setCurrentWidget(UMLWidget* currentWidget);

    /**
     * Overriden from base class to do nothing, as arrow is the default tool.
     */
    virtual void changeTool();

    /**
     * The selection rectangle that contains the four lines of its borders.
     */
    QPtrList<QCanvasLine> m_selectionRect;

    /**
     * The start position of the selection rectangle.
     */
    QPoint m_startPosition;

};

#endif //TOOLBARSTATEARROW_H
