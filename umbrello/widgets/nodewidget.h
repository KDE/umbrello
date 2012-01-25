/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef NODEWIDGET_H
#define NODEWIDGET_H

#include "umlwidget.h"

class UMLNode;

/**
 * Defines a graphical version of the Node used in Deployment diagram.
 * Most of the functionality will come from the @ref UMLNode class.
 *
 * @short A graphical version of a Node.
 * @author Jonathan Riddell
 * @author Gopala Krishna
 * @see UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class NodeWidget : public UMLWidget
{
public:
    NodeWidget(UMLNode *n );
    virtual ~NodeWidget();

    virtual void paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w);

    // Uses UMLWidget::loadFromXMI to load info.
    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

protected:
    virtual void updateGeometry();
    virtual void updateTextItemGroups();
    virtual QVariant attributeChange(WidgetAttributeChange change, const QVariant& oldValue);

private:
    static const qreal DEPTH;  ///< pixels on Z axis
    enum {
        GroupIndex
    };
    enum {
        StereoItemIndex,
        NameItemIndex,
        TextItemCount
    };

    /// Path representing the drawing of node widget.
    QPainterPath m_nodeWidgetPath;
};

#endif
