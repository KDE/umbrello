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

#ifndef TOOLBARSTATEOTHER_H
#define TOOLBARSTATEOTHER_H

#include "toolbarstatepool.h"

/**
 * Other tool creates almost all the objects (except associations and messages).
 * Objects are created when left button is released, no matter if it was
 * released on an association, on a widget or on an empty space.
 *
 * Associations and widgets aren't taken into account, and are treated as empty
 * spaces.
 */
class ToolBarStateOther : public ToolBarStatePool {
    Q_OBJECT
public:

    /**
     * Creates a new ToolBarStateOther.
     *
     * @param umlView The UMLView to use.
     */
    ToolBarStateOther(UMLView *umlView);

    /**
     * Destroys this ToolBarStateOther.
     */
    virtual ~ToolBarStateOther();

private:

    /**
     * Sets nothing.
     * Overriden from base class to ignore associations and widgets and treat
     * them as empty spaces to create widgets on it.
     */
    virtual void setCurrentElement();

    /**
     * Called when the release event happened on an empty space.
     * Associations, widgets and actual empty spaces are all treated as empty
     * spaces. It creates a new widget if the left button was released.
     * The widget to create depends on the type of the toolbar button selected.
     * If the widget is the visual representation of an UMLObject, the object
     * factory handles its creation. Otherwise, the widget is created using
     * newWidget().
     * The UMLView is resized to fit on all the items.
     */
    virtual void mouseReleaseEmpty();

    /**
     * Returns the object type of this tool.
     *
     * @return The object type of this tool.
     */
    Uml::Object_Type getObjectType();

    /**
     * Creates and adds a new widget to the UMLView (if widgets of that type
     * don't have an associated UMLObject).
     * If the type of the widget doesn't use an UMLObject (for example, a note
     * or a box), it creates the widget, adds it to the view and returns true.
     * Otherwise, it returns false.
     *
     * @return True if the widget was created, false otherwise.
     * @todo rename to something more clear
     */
    bool newWidget();

};

#endif //TOOLBARSTATEOTHER_H
