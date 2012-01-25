/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef DATATYPEWIDGET_H
#define DATATYPEWIDGET_H

#include "umlwidget.h"

class UMLClassifier;

/**
 * @class DatatypeWidget
 *
 * Defines a graphical version of the datatype.  Most of the
 * functionality will come from the @ref UMLWidget class from
 * which class inherits from.
 *
 * @short A graphical version of an datatype.
 * @author Jonathan Riddell
 * @author Gopala Krishna
 *
 * @see UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class DatatypeWidget : public UMLWidget
{
public:
    DatatypeWidget(UMLClassifier *d);
    virtual ~DatatypeWidget();

    // Uses UMLWidget::loadFromXMI to load data from XMI
    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

    virtual void paint(QPainter *p, const QStyleOptionGraphicsItem *opt, QWidget *w);

protected:
    virtual void updateGeometry();
    virtual void updateTextItemGroups();
    virtual QVariant attributeChange(WidgetAttributeChange change, const QVariant& old);

private:
    enum {
        GroupIndex = 0
    };
    enum {
        StereoTypeItemIndex = 0,
        NameItemIndex,
        TextItemCount
    };
};

#endif
