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
 * object is serialized. With this class we are trying to achieve
 * isolation between data and display layers.
 *
 * @author Jonathan Riddell
 */
class ComponentWidgetData : public UMLWidgetData {
	friend class ComponentWidget;
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
	 * Save/Restore the object for the clipboard.
	 *	@param	s	Pointer to the data stream (file) to save/load from.
	 *	@param	archive	If true will save the object information else will load the information
	 *	@param	fileversion	the version of the serialize format
	 *	@return	true if successful else false
	 */
	virtual bool serialize(QDataStream* s, bool archive, int fileversion);

	/**
	 * Returns the amount of bytes needed to serialize this object
	 * If the serialization method of this class is changed this function will have to be CHANGED TOO
	 * This function is used by the Copy and Paste Functionality
	 * The Size in bytes of a serialized QString Object is long sz:
	 *  if ( (sz =str.length()*sizeof(QChar)) && !(const char*)str.unicode() ) {
	 *      sz = size of Q_UINT32; //  typedef unsigned int	Q_UINT32;		// 32 bit unsigned
	 *  }
	 * This calculation is valid only for QT 2.1.x or superior,
	 * this is totally incompatible with QT 2.0.x or QT 1.x or
	 * inferior
	 * That means the copy and paste functionality will work on with QT 2.1.x or superior
	 */
	virtual long getClipSizeOf();

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
