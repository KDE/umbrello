/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef COMPONENTWIDGET_H
#define COMPONENTWIDGET_H

#include "umlwidget.h"

class UMLComponent;

#define COMPONENT_MARGIN 10

/**
 * Defines a graphical version of the Component.  Most of the functionality
 * will come from the @ref UMLComponent class.
 *
 * @short A graphical version of a Component.
 * @author Jonathan Riddell
 * @see UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ComponentWidget : public UMLWidget {
public:

    /**
     * Constructs a ComponentWidget.
     *
     * @param view      The parent of this ComponentWidget.
     * @param c The UMLComponent this will be representing.
     */
    ComponentWidget(UMLView * view, UMLComponent *c);

    /**
     * destructor
     */
    virtual ~ComponentWidget();

    /**
     * Overrides standard method
     */
    void draw(QPainter& p, int offsetX, int offsetY);

    /**
     * Saves to the "componentwidget" XMI element.
     */
    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

protected:
    /**
     * Overrides method from UMLWidget.
     */
    QSize calculateSize();

private:
    /**
     * Initializes key variables of the class.
     */
    void init();

    /**
     * The right mouse button menu
     */
    ListPopupMenu* m_pMenu;
};

#endif
