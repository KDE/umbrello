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
 *	when an ObjectWidget object is serialized
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
	 * Returns the amount of bytes needed to serialize this object
	 * If the serialization method of this class is changed this function will have to be CHANGED TOO
	 * This function is used by the Copy and Paste Functionality
	 * The Size in bytes of a serialized QString Object is long sz:
	 *	if ( (sz =str.length()*sizeof(QChar)) && !(const char*)str.unicode() )
	 *	{
	 *		sz = size of Q_UINT32; //  typedef unsigned int	Q_UINT32;		// 32 bit unsigned
	 *	}
	 * This calculation is valid only for QT 2.1.x or superior,
	 * this is totally incompatible with QT 2.0.x or QT 1.x or
	 * inferior
	 * That means the copy and paste functionality will work on with QT 2.1.x or superior
	 */
	virtual long getClipSizeOf();

	/**
	 * 		Save/Loads the class.
	 */
	virtual bool serialize(QDataStream *s, bool archive, int fileversion);

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
