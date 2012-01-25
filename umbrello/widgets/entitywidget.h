/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef ENTITYWIDGET_H
#define ENTITYWIDGET_H

#include "umlwidget.h"

/**
 * Defines a graphical version of the entity.  Most of the functionality
 * will come from the @ref UMLWidget class from which class inherits from.
 *
 * @short A graphical version of an entity.
 * @author Jonathan Riddell
 * @author Gopala Krishna
 * @see UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class EntityWidget : public UMLWidget
{
    Q_OBJECT
public:
    explicit EntityWidget(UMLObject* o);
    virtual ~EntityWidget();

    virtual void paint(QPainter *p, const QStyleOptionGraphicsItem *item, QWidget *w);

    // UMLWidget::loadFromXMI is used to load this widget.
    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

public Q_SLOTS:
    virtual void slotMenuSelection(QAction* action);

protected:
    virtual void updateGeometry();
    virtual void updateTextItemGroups();
    virtual QVariant attributeChange(WidgetAttributeChange change, const QVariant& oldValue);


private:

    // Hardcoded indices for text items
    enum {
        GroupIndex
    };
    enum {
        StereotypeItemIndex,
        NameItemIndex,
        EntityItemStartIndex
    };

    QLineF m_nameLine;
};

#endif
