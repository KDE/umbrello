/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef COMPONENTWIDGET_H
#define COMPONENTWIDGET_H

#include "umlwidget.h"

class UMLView;

#define COMPONENT_MARGIN 10

/**
 *	Defines a graphical version of the Component.  Most of the functionality
 *	will come from the @ref UMLComponent class.
 *
 *	@short	A graphical version of a Component.
 *	@author Jonathan Riddell
 *	@see	UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ComponentWidget : public UMLWidget {
public:

	/**
	 *	Constructs a ComponentWidget.
	 *
	 *	@param	view	The parent of this ComponentWidget.
	 *	@param	o	The UMLObject this will be representing.
	 */
	ComponentWidget(UMLView * view, UMLObject * o);

	/**
	 *	Constructs a ComponentWidget.
	 *
	 *	@param	view	The parent of this ComponentWidget.
	 */
	ComponentWidget(UMLView * view);

	/**
	 *	Standard deconstructor.
	 */
	~ComponentWidget();

	/**
	 *	Initializes key variables of the class.
	 */
	void init();

	/**
	 *	Returns the status of whether to show StereoType.
	 *
	 *	@return  Returns the status of whether to show StereoType.
	 */
	bool getShowStereotype() const;

	/**
	 * 	Set the status of whether to show StereoType
	 *
	 *	@param _show  The status of whether to show StereoType
	 */
	void setShowStereotype(bool _status);

	/**
	 * Activate the object after serializing it from a QDataStream
	 */
	virtual bool activate(IDChangeLog* ChangeLog  = 0 );

	/**
	 * Overrides standard method
	 */
	void draw(QPainter& p, int offsetX, int offsetY);

	/**
	 * Saves to the <componentwidget> element
	 */
	bool saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

	/**
	 * Loads from a <componentwidget> element
	 */
	bool loadFromXMI(QDomElement& qElement);

protected:
	/**
	 * Should it show the <<stereotype>> of the component, currently ignored
	 * (stereotype is shown if it isn't empty).
	 */
	bool m_bShowStereotype;

private:
	/**
	 *	Automatically calculates the size of the object.
	 */
	void calculateSize();

	/**
	 * The right mouse button menu
	 */
	ListPopupMenu* m_pMenu;
};

#endif
