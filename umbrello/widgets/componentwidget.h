/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
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
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class ComponentWidget : public UMLWidget 
{
public:
    ComponentWidget(UMLScene * scene, UMLComponent *c);
    virtual ~ComponentWidget();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

protected:
    QSizeF minimumSize();

};

#endif
