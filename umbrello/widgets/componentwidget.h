/***************************************************************************
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
class ComponentWidget : public UMLWidget 
{
public:
    ComponentWidget(UMLScene * scene, UMLComponent *c);
    virtual ~ComponentWidget();

    virtual void paint(QPainter& p, int offsetX, int offsetY);

    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

protected:
    UMLSceneSize minimumSize();
    /**
     * The right mouse button menu
     */
    ListPopupMenu* m_pMenu;
};

#endif
