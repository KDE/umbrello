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

#ifndef TOOLBARSTATEONEWIDGET_H
#define TOOLBARSTATEONEWIDGET_H

#include "toolbarstatepool.h"
//Added by qt3to4:
#include <QMouseEvent>
#include <qpoint.h>

class Q3CanvasLine;
class ObjectWidget;
class UMLWidget;

/**
 * Sequence tool to create components linked with one object in sequence diagram 
 * like precondition.
 * With sequence tool, one objects is selected clicking with left button on
 * it
 */

class ToolBarStateOneWidget : public ToolBarStatePool {
    Q_OBJECT
public:

    /**
     * Creates a new ToolBarStateOneWidget.
     *
     * @param umlView The UMLView to use.
     */
    ToolBarStateOneWidget(UMLView *umlView);

    /**
     * Destroys this ToolBarStateOneWidget.
     */
    virtual ~ToolBarStateOneWidget();

    /**
     * Goes back to the initial state.
     */
    virtual void init();

    /**
     * Called when the current tool is changed to use another tool.
     * Executes base method and cleans the message.
     */
    virtual void cleanBeforeChange();

    /**
     * Called when a mouse event happened.
     * It executes the base method and then updates the position of the
     * message line, if any.
     */
    virtual void mouseMove(QMouseEvent* ome);

public slots:

    /**
     * A widget was removed from the UMLView.
     * If the widget removed was the current widget, the current widget is set
     * to 0.
     * Also, if it was the first object, the message is cleaned.
     */
    virtual void slotWidgetRemoved(UMLWidget* widget);

protected:

    /**
     * Selects only widgets, but no associations.
     * Overrides base class method.
     * If the press event happened on the line of an object, the object is set
     * as current widget. If the press event happened on a widget, the widget is
     * set as current widget.
     */
    virtual void setCurrentElement();

    /**
     * Called when the release event happened on a widget.
     * If the button pressed isn't left button or the widget isn't an object
     * widget, the message is cleaned.
     * If the release event didn't happen on the line of an object and the first
     * object wasn't selected, nothing is done. If the first object was already
     * selected, a creation message is made.
     * If the event happened on the line of an object, the first object or the
     * second are set, depending on whether the first object was already set or
     * not.
     */
    virtual void mouseReleaseWidget();

    /**
     * Called when the release event happened on an empty space.
     * Cleans the message.
     * Empty spaces are not only actual empty spaces, but also associations.
     */
    virtual void mouseReleaseEmpty();

protected:

    /**
     * Sets the first object of the message using the specified object.
     * The temporal visual message is created and mouse tracking enabled, so
     * mouse events will be delivered.
     *
     * @param firstObject The first object of the message.
     */
    void setWidget(UMLWidget* firstObject);

 
    /**
     * Returns the widget type of this tool.
     *
     * @return The widget type of this tool.
     */
    Uml::Widget_Type getWidgetType();


    /**
     * The first object in the message.
     */
    UMLWidget* m_firstObject;

    UMLView * m_umlView;
    /**
     * If there is a current widget, it is true if the press event happened on
     * the line of an object, or false if it happened on a normal UMLWidget.
     */
    bool m_isObjectWidgetLine;

};

#endif //TOOLBARSTATEONEWIDGET_H
