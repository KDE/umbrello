/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PACKAGEWIDGETDATA_H
#define PACKAGEWIDGETDATA_H

#include "umlwidgetdata.h"

/**
 * This class holds all the PackageWidget's Information. All this
 * information goes to a file or clipboard when an ConceptWidget
 * object is saved. With this class we are trying to achieve
 * isolation between data and display layers.
 *
 * @author Jonathan Riddell
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class PackageWidgetData : public UMLWidgetData {
public:

	/**
	 * creates a PackageWidgetData object
	 */
	PackageWidgetData(SettingsDlg::OptionState optionState);

	/**
	 * creates a copy of a PackageWidgetData object
	 */
	PackageWidgetData(PackageWidgetData& Other);

	/**
	 * destructor, empty
	 */
	virtual ~PackageWidgetData();

	/**
	 * Make the same as another PackageWidgetData
	 */
	virtual PackageWidgetData& operator=(PackageWidgetData& Other);

	/**
	 * Compare two PackageWidgetData.
	 */
	virtual bool operator==(PackageWidgetData& Other);

	/**
	 * Write property of bool m_bShowStereotype.
	 */
	virtual void setShowStereotype( bool ShowStereotype);

	/**
	 * Read property of bool m_bShowStereotype.
	 */
	virtual bool getShowStereotype();

	/**
	 * Debugging method
	 */
	virtual void print2cerr();

	/**
	 * Saves to the <packagewidget> element
	 */
	bool saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

	/**
	 * Loads from a <packagewidget> element
	 */
	bool loadFromXMI(QDomElement& qElement);
protected:

	/**
	 * Should it show the <<stereotype>> of the package, currently ignored
	 * (stereotype is shown if it isn't empty).
	 */
	bool m_bShowStereotype;
};

#endif
