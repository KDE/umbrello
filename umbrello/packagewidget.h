/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PACKAGEWIDGET_H
#define PACKAGEWIDGET_H

#include "packagewidgetdata.h"
#include "umlwidget.h"

class UMLView;

#define PACKAGE_MARGIN 5

/**
 *	Defines a graphical version of the Package.  Most of the functionality
 *	will come from the @ref UMLPackage class.
 *
 *	@short	A graphical version of a Package.
 *	@author Jonathan Riddell
 *	@see	UMLWidget
 */
class PackageWidget : public UMLWidget {
public:
	/**
	 *	Constructs a PackageWidget.
	 *
	 *	@param	view		The parent of this PackageWidget.
	 *	@param	pData		The PackageWidgetData to represent.
	 */
	PackageWidget(UMLView* view, UMLObject* o, UMLWidgetData* pData);

	/**
	 *	Constructs a PackageWidget.
	 *
	 *	@param	view		The parent of this PackageWidget.
	 *	@param	o		The UMLObject is will be representing.
	 */
	PackageWidget(UMLView* view, UMLObject* o);

	/**
	 *	Constructs a PackageWidget.
	 *
	 *	@param	view	The parent of this PackageWidget.
	 */
	PackageWidget(UMLView* view);

	/**
	 *	Standard deconstructor.
	 */
	~PackageWidget();

	/**
	 *	Initializes key variables of the class.
	 */
	void init();

	/**
	 *	Returns the status of whether to show StereoType.
	 *
	 *	@return  Returns the status of whether to show StereoType.
	 */
	bool getShowStereotype();

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
	 *	Overrides standards method
	 */
	void draw(QPainter& p, int offsetX, int offsetY);
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
