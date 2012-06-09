/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef PRECONDITIONWIDGET_H
#define PRECONDITIONWIDGET_H

#include "umlwidget.h"

class ObjectWidget;

/**
 * @short  A graphical version of a UML Precondition (new in UML 2.0).
 *
 * This class is attached to sequential line of an ObjectWidget and
 * represents a precondition. (Sequence diagram)
 *
 * @author Florence Mattler <florence.mattler@libertysurf.fr>
 * @author Gopala Krishna
 *
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class PreconditionWidget : public UMLWidget
{
    Q_OBJECT
public:
    PreconditionWidget(ObjectWidget* a, Uml::IDType id = Uml::id_None);
    virtual ~PreconditionWidget();

    qreal minY() const;
    qreal maxY() const;

    virtual void paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *w);

    void alignToObjectLine();

    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);
    virtual bool loadFromXMI(QDomElement& qElement);

public Q_SLOTS:
    virtual void slotMenuSelection(QAction* action);

protected:
    virtual void updateGeometry();
    virtual void updateTextItemGroups();
    virtual QVariant attributeChange(WidgetAttributeChange change, const QVariant& oldValue);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value);

private:
    ObjectWidget* m_objectWidget;

};

#endif
