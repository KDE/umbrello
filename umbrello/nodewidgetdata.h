/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef NODEWIDGETDATA_H
#define NODEWIDGETDATA_H

#include "umlwidgetdata.h"

/**
 * This class holds all the NodeWidget's Information. All this
 * information goes to a file or clipboard when an ConceptWidget
 * object is saved. With this class we are trying to achieve
 * isolation between data and display layers.
 *
 * @author Jonathan Riddell
 */
class NodeWidgetData : public UMLWidgetData {
	friend class NodeWidget;
public:

	/**
	 * creates a NodeWidgetData object
	 */
	NodeWidgetData(SettingsDlg::OptionState optionState);

	/**
	 * creates a copy of a NodeWidgetData object
	 */
	NodeWidgetData(NodeWidgetData& Other);

	/**
	 * destructor, empty
	 */
	virtual ~NodeWidgetData();

	/**
	 * Make the same as another NodeWidgetData
	 */
	virtual NodeWidgetData& operator=(NodeWidgetData& Other);

	/**
	 * Compare two NodeWidgetData.
	 */
	virtual bool operator==(NodeWidgetData& Other);

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
	 * Saves to the <nodewidget> element
	 */
	bool saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

	/**
	 * Loads from a <nodewidget> element
	 */
	bool loadFromXMI(QDomElement& qElement);
protected:

	/**
	 * Should it show the <<stereotype>> of the node, currently ignored
	 * (stereotype is shown if it isn't empty).
	 */
	bool m_bShowStereotype;
};

#endif
