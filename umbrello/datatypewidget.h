/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DATATYPEWIDGET_H
#define DATATYPEWIDGET_H

#include "umlwidget.h"

class UMLView;

#define DATATYPE_MARGIN 5

/**
 * Defines a graphical version of the datatype.  Most of the functionality
 * will come from the @ref UMLWidget class from which class inherits from.
 *
 * @short A graphical version of an datatype.
 * @author Jonathan Riddell
 * @see	UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class DatatypeWidget : public UMLWidget {
public:

	/**
	 * Constructs an DatatypeWidget.
	 *
	 * @param view		The parent of this DatatypeWidget.
	 * @param o		The UMLObject this will be representing.
	 */
	DatatypeWidget(UMLView* view, UMLObject* o);

	/**
	 * Constructs an DatatypeWidget.
	 *
	 * @param view		The parent of this DatatypeWidget.
	 */
	DatatypeWidget(UMLView* view);

	/**
	 * Standard deconstructor.
	 */
	~DatatypeWidget();

	/**
	 * Initializes key variables of the class.
	 */
	void init();

	/**
	 * Activate the object after serializing it from a QDataStream.
	 */
	virtual bool activate(IDChangeLog* ChangeLog  = 0 );

	/**
	 * Overrides standard method.
	 */
	void draw(QPainter& p, int offsetX, int offsetY);

	/**
	 * Saves to the <UML:DatatypeWidget> XMI element.
	 */
	bool saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

	/**
	 * Loads from an <UML:DatatypeWidget> XMI element.
	 */
	bool loadFromXMI(QDomElement& qElement);

protected:

private:
	/**
	 * Automatically calculates the size of the object.
	 */
	void calculateSize();

	/**
	 * The right mouse button menu.
	 */
	ListPopupMenu* m_pMenu;

public slots:
};

#endif
