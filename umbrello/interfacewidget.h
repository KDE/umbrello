 /*
  *  copyright (C) 2003-2004
  *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
  */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef INTERFACEWIDGET_H
#define INTERFACEWIDGET_H

#include "classifierwidget.h"

class UMLInterface;

/**
 * Defines a graphical version of the interface.  Most of the functionality
 * comes from its ancestors, @ref ClassifierWidget and @ref UMLWidget.
 *
 * @short A graphical version of an interface.
 * @author Jonathan Riddell
 * @see	UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class InterfaceWidget : public ClassifierWidget {
public:

	/**
	 * Constructs an InterfaceWidget.
	 *
	 * @param view		The parent of this InterfaceWidget.
	 * @param i		The UMLInterface this will be representing.
	 */
	InterfaceWidget(UMLView * view, UMLInterface *i);

	/**
	 * destructor
	 */
	virtual ~InterfaceWidget();

	/**
	 * Returns the status of whether to draw as circle.
	 *
	 * @return	True if widget is drawn as circle.
	 */
	bool getDrawAsCircle() const;

	/**
	 * Toggles the status of whether to draw as circle.
	 */
	void toggleDrawAsCircle();

	/**
	 * Set the status of whether to draw as circle.
	 *
	 * @param _show		True if widget shall be drawn as circle.
	 */
	void setDrawAsCircle(bool drawAsCircle);

	/**
	 * Overrides standard method.
	 */
	void draw(QPainter& p, int offsetX, int offsetY);

	/**
	 * Overrides default impl. to set e.g. the abstract attribute.
	 *
	 * @param object	Pointer to the UMLObject that is associated
	 *			to this widget.
	 */
	virtual void setUMLObject(UMLObject* object);

	/**
	 * Saves to the <interfacewidget> XMI element.
	 */
	void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 * Loads from an <interfacewidget> XMI element.
	 */
	bool loadFromXMI( QDomElement & qElement );

protected:
	bool m_bDrawAsCircle;   ///< Loaded/saved item.

private:
	/**
	 * Initializes key variables of the class.
	 */
	void init();

	/**
	 * draws the interface as a circle with name underneath.
	 */
	void drawAsCircle(QPainter& p, int offsetX, int offsetY);

	/**
	 * Automatically calculates the size of the object.
	 */
	void calculateSize();

	/**
	 * Automatically calculates the size of the object when drawn as
	 * a circle.
	 */
	void calculateAsCircleSize();

	/**
	 * Automatically calculates the size of the object when drawn as
	 * a concept.
	 */
	void calculateAsConceptSize();

	/**
	 * Updates m_ShowOpSigs to match m_bShowScope.
	 */
	void updateSigs();

public slots:
	/**
	 * Will be called when a menu selection has been made from the
	 * popup menu.
	 *
	 * @param sel	The selection id that has been selected.
	 */
	void slotMenuSelection(int sel);
};

#endif
