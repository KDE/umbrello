/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef COMPONENTWIDGETDATA_H
#define COMPONENTWIDGETDATA_H

#include "umlwidgetdata.h"

/**
 * This class holds all the ComponentWidget's Information. All this
 * information goes to a file or clipboard when an ConceptWidget
 * object is saved. With this class we are trying to achieve
 * isolation between data and display layers.
 *
 * @author Jonathan Riddell
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ComponentWidgetData : public UMLWidgetData {
public:

	/**
	 * creates a ComponentWidgetData object
	 */
	ComponentWidgetData(SettingsDlg::OptionState optionState);

	/**
	 * creates a copy of a ComponentWidgetData object
	 */
	ComponentWidgetData(ComponentWidgetData& Other);

	/**
	 * destructor, empty
	 */
	virtual ~ComponentWidgetData();

	/**
	 * Make the same as another ComponentWidgetData
	 */
	virtual ComponentWidgetData& operator=(ComponentWidgetData& Other);

	/**
	 * Compare two ComponentWidgetData.
	 */
	virtual bool operator==(ComponentWidgetData& Other);

	/**
	 * Write property of bool m_bShowStereotype.
	 */
	virtual void setShowStereotype(bool showStereotype);

	/**
	 * Read property of bool m_bShowStereotype.
	 */
	virtual bool getShowStereotype();

	/**
	 * Debugging method
	 */
	virtual void print2cerr();

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
};

#endif
