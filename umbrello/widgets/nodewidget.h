/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
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
 * @see UMLWidget
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class NodeWidget : public UMLWidget
{
public:

    /**
     * Constructs a NodeWidget.
     *
     * @param scene              The parent of this NodeWidget.
     * @param n         The UMLNode this will be representing.
     */
    NodeWidget(UMLScene * scene, UMLNode *n);

    /**
     * destructor
     */
    virtual ~NodeWidget();

    /**
     * Overrides standard method.
     */
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    /**
     * Saves to the "nodewidget" XMI element.
     * Note: For loading we use the method inherited from UMLWidget.
     */
    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

protected:
    /**
     * Overrides method from UMLWidget
     */
    UMLSceneSize minimumSize();

    static const int DEPTH = 30;  ///< pixels on Z axis
};

#endif
