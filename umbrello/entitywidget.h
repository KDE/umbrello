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

#ifndef ENTITYWIDGET_H
#define ENTITYWIDGET_H

#include "newumlrectwidget.h"

/**
 * Defines a graphical version of the entity.  Most of the functionality
 * will come from the @ref NewUMLRectWidget class from which class inherits from.
 *
 * @short A graphical version of an entity.
 * @author Jonathan Riddell
 * @author Gopala Krishna
 * @see NewUMLRectWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class EntityWidget : public NewUMLRectWidget
{
	Q_OBJECT
public:
	explicit EntityWidget(UMLObject* o);
	virtual ~EntityWidget();

	virtual void paint(QPainter *p, const QStyleOptionGraphicsItem *item, QWidget *w);

    // NewUMLRectWidget::loadFromXMI is used to load this widget.
    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

protected:
	virtual void updateGeometry();
	virtual void updateTextItemGroups();
	virtual QVariant attributeChange(WidgetAttributeChange change, const QVariant& oldValue);

public Q_SLOTS:
    virtual void slotMenuSelection(QAction* action);

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
