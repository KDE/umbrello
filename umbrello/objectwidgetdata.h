/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef OBJECTWIDGETDATA_H
#define OBJECTWIDGETDATA_H

#include <qstring.h>
#include "umlwidgetdata.h"
#include "dialogs/settingsdlg.h"

/**	This class holds all the ObjectWidget's Information
 *	All this information goes to a file or clipboard
 *	when an ObjectWidget object is saved
 *	With this class we are trying to achieve isolation
 *	between data and display layers.
 *	@author Gustavo Madrigal
 */

class ObjectWidgetData : public UMLWidgetData {
	friend class ObjectWidget;
public:

	/**
	 * 		Constructor
	 */
	ObjectWidgetData(SettingsDlg::OptionState optionState);

	/**
	 * 		Constructor
	 */
	ObjectWidgetData(ObjectWidgetData & Other);

	/**
	 * 		Deconstructor
	 */
	virtual ~ObjectWidgetData();

	/**
	 * 		Copy constructor
	 */
	virtual ObjectWidgetData & operator=(ObjectWidgetData & Other);

	/**
	 * 		Overloaded "==" operator
	 */
	virtual bool operator==(ObjectWidgetData & Other);

	/**
	 * Write property of QString m_InstanceName.
	 */
	virtual void setInstanceName( QString InstanceName);

	/**
	 * Read property of QString m_InstanceName.
	 */
	virtual QString getInstanceName();

	/**
	 *	 Write property of int m_nLocalID.
	 */
	virtual void setLocalID( int LocalID);

	/**
	 *	Read property of int m_nLocalID.
	 */
	virtual int getLocalID();

	/**
	 * Write property of bool m_bMultipleInstance.
	 */
	virtual void setMultipleInstance( bool MultipleInstance);

	/**
	 * Read property of bool m_bMultipleInstance.
	 */
	virtual bool getMultipleInstance();

	/**
	 * Prints the data members to standard error
	 */
	virtual void print2cerr();

	/**
	 * 		Returns whether to draw as an object or not.
	 */
	bool getDrawAsActor() {
		return m_bDrawAsActor;
	}

	/**
	 * 		Sets whether to draw as an Actor.
	 */
	void setDrawAsActor( bool drawAsActor ) {
		m_bDrawAsActor = drawAsActor;
	}

	/**
	 *		Sets whether to show deconstruction on sequence line
	 */
	void setShowDeconstruction( bool bShow ) {
		m_bShowDeconstruction = bShow;
	}

	/**
	 *		Returns whether to show deconstruction on sequence line
	 */
	bool getShowDeconstruction() {
		return m_bShowDeconstruction;
	}

	/**
	 *		Sets the length of the line
	 */
	void setLineLength( int nLength ) {
		m_nLineLength = nLength;
	}

	/**
	 *		Sets the length of the line
	 */
	int getLineLength() {
		return m_nLineLength;
	}

	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	bool loadFromXMI( QDomElement & qElement );

protected:

	/**
	 *		Instance name of object.
	 */
	QString m_InstanceName;

	/**
	 *		Local ID used on views.  Needed as a it can
	 *		represent a class that has many objects
	 *		representing it.
	 */
	int m_nLocalID;

	/**
	 *		Determines whether to draw an object as a
	 *		multiple object instance.
	 */
	bool m_bMultipleInstance;

	/**
	 *		Determines whether the object should be drawn
	 *		as an Actor or a an Object.
	 */
	bool m_bDrawAsActor;

	/**
	 *		Determines whether to show object destruction
	 *		on sequence diagram line.
	 */
	bool m_bShowDeconstruction;

	/**
	 *		Length of seq line.
	 */
	int m_nLineLength;
};

#endif
