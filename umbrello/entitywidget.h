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

#ifndef ENTITYWIDGET_H
#define ENTITYWIDGET_H

#include "umlwidget.h"

class UMLView;

#define ENTITY_MARGIN 5

/**
 * Defines a graphical version of the entity.  Most of the functionality
 * will come from the @ref UMLWidget class from which class inherits from.
 *
 * @short A graphical version of an entity.
 * @author Jonathan Riddell
 * @see UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class EntityWidget : public UMLWidget {
public:

    /**
     * Constructs an EntityWidget.
     *
     * @param view              The parent of this EntityWidget.
     * @param o         The UMLObject this will be representing.
     */
    EntityWidget(UMLView* view, UMLObject* o);

    /**
     * Standard deconstructor.
     */
    ~EntityWidget();

    /**
     * Initializes key variables of the class.
     */
    void init();

    /**
     * Draws the entity as a rectangle with a box underneith with a list of literals
     */
    void draw(QPainter& p, int offsetX, int offsetY);

    /**
     * Saves to the "entitywidget" XMI element.
     */
    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

    /**
     * Loads from an "entitywidget" XMI element.
     */
    bool loadFromXMI(QDomElement& qElement);

protected:
    /**
     * Overrides method from UMLWidget.
     */
    QSize calculateSize();

public slots:
    /**
     * Will be called when a menu selection has been made from the
     * popup menu.
     *
     * @param sel       The selection id that has been selected.
     */
    void slotMenuSelection(int sel);
};

#endif
