/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef COMPONENTWIDGET_H
#define COMPONENTWIDGET_H

#include "umlwidget.h"

class UMLComponent;

/**
 * Defines a graphical version of the Component.  Most of the functionality
 * will come from the @ref UMLComponent class.
 *
 * @short A graphical version of a Component.
 * @author Jonathan Riddell
 * @author Gopala Krishna
 * @see UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ComponentWidget : public UMLWidget
{
public:
    ComponentWidget(UMLComponent *c);
    virtual ~ComponentWidget();

    virtual void paint(QPainter *p, const QStyleOptionGraphicsItem *item, QWidget *w);

    // Uses UMLWidget::loadFromXMI to load info.
    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

protected:
    virtual void updateGeometry();
    virtual void updateTextItemGroups();
    virtual QVariant attributeChange(WidgetAttributeChange change, const QVariant& oldValue);

private:
    // Hard coded indices used for TextItemGroup and TextItems.
    enum {
        GroupIndex
    };
    enum {
        StereoItemIndex,
        NameItemIndex,
        TextItemCount
    };

    /// The rectangles to be drawn (calculated in attributeChange)
    QRectF m_rects[3];
};

#endif
