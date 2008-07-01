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

#ifndef NODEWIDGET_H
#define NODEWIDGET_H

#include "umlwidget.h"

class UMLNode;

/**
 * Defines a graphical version of the Node.  Most of the functionality
 * will come from the @ref UMLNode class.
 *
 * @short A graphical version of a Node.
 * @author Jonathan Riddell
 * @see NewUMLRectWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class NodeWidget : public NewUMLRectWidget {
public:

    /**
     * Constructs a NodeWidget.
     *
     * @param view              The parent of this NodeWidget.
     * @param n         The UMLNode this will be representing.
     */
    NodeWidget(UMLScene * view, UMLNode *n );

    /**
     * destructor
     */
    virtual ~NodeWidget();

    /**
     * Overrides standard method.
     */
    void paint(QPainter *p, const QStyleOptionGraphicsItem *item, QWidget *w);

    /**
     * Saves to the "nodewidget" XMI element.
     * Note: For loading we use the method inherited from NewUMLRectWidget.
     */
    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

protected:
    /**
     * Overrides method from NewUMLRectWidget
     */
    QSizeF calculateSize();

    static const qreal DEPTH = 30;  ///< pixels on Z axis
};

#endif
