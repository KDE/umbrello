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

// Forward declarations
class TextItemGroup;

/**
 * Defines a graphical version of the entity.  Most of the functionality
 * will come from the @ref NewUMLRectWidget class from which class inherits from.
 *
 * @short A graphical version of an entity.
 * @author Jonathan Riddell
 * @author Gopala Krishna (port using TextItems)
 * @see NewUMLRectWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class EntityWidget : public NewUMLRectWidget
{
	Q_OBJECT
public:
	EntityWidget(UMLObject* o);
	~EntityWidget();

	QSizeF sizeHint(Qt::SizeHint which);
	void paint(QPainter *p, const QStyleOptionGraphicsItem *item, QWidget *w);

    // NewUMLRectWidget::loadFromXMI is used to load this widget.
    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

protected:
	void updateGeometry();
	void sizeHasChanged(const QSizeF& oldSize);

public Q_SLOTS:
    void slotMenuSelection(QAction* action);

private:
	static const qreal Margin;
	// Indices for text items in m_textItemGroup
	enum {
		StereotypeItemIndex,
		NameItemIndex,
		EntityItemStartIndex
	};

	QSizeF m_minimumSize;
	TextItemGroup *m_textItemGroup;
};

#endif
