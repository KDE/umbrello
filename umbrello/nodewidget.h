/*
 *  copyright (C) 2003-2004
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef NODEWIDGET_H
#define NODEWIDGET_H

#include "resizablewidget.h"

class UMLNode;

/**
 * Defines a graphical version of the Node.  Most of the functionality
 * will come from the @ref UMLNode class.
 *
 * @short A graphical version of a Node.
 * @author Jonathan Riddell
 * @see UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class NodeWidget : public ResizableWidget {
public:

    /**
     * Constructs a NodeWidget.
     *
     * @param view              The parent of this NodeWidget.
     * @param o         The UMLNode this will be representing.
     */
    NodeWidget(UMLView * view, UMLNode *n );

    /**
     * destructor
     */
    virtual ~NodeWidget();

    /**
     * Overrides standard method.
     */
    void draw(QPainter& p, int offsetX, int offsetY);

    /**
     * Overrides method from ResizableWidget.
     */
    void constrain(int& width, int& height);

    /**
     * Saves to the <nodewidget> XMI element.
     * Note: For loading we use the method inherited from UMLWidget.
     */
    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

protected:
    /**
     * Automatically calculates the size of the object.
     */
    void calculateSize();

    /**
     * Compute the minimum possible width and height.
     *
     * @param width  return value, computed width
     * @param height return value, computed height
     */
    void calcMinWidthAndHeight(int& width, int& height);

    static const int DEPTH = 30;  ///< pixels on Z axis
};

#endif
