/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ACTIVITYWIDGETDATA_H
#define ACTIVITYWIDGETDATA_H

#include "umlwidgetdata.h"
#include "activitywidget.h"

/**
 * This class holds all the ActivityWidget's Information All this
 * information goes to a file or clipboard when an ActivityWidget
 * object is serialized With this class we are trying to achieve
 * isolation between data and display layers.
 *   @author Paul Hensgen
 */
class ActivityWidgetData : public UMLWidgetData {
public:

	/**
	*   Default constructor
	*/
	ActivityWidgetData(SettingsDlg::OptionState optionState);

	/**
	*   Copy constructor
	*/
	ActivityWidgetData(ActivityWidgetData & Other);

	/**
	*   Deconstructor
	*/
	virtual ~ActivityWidgetData();

	/**
	*   Overrides '=' operator
	*/
	virtual ActivityWidgetData & operator=(ActivityWidgetData & Other);

	/**
	*   Overrides '==' operator
	*/
	virtual bool operator==(ActivityWidgetData & Other);

	/**
	*   Returns the buffer size needed to serialize object
	*/
	virtual long getClipSizeOf();

	/**
	*   Saves/Loads the object.
	*/
	virtual bool serialize(QDataStream *s, bool archive, int fileversion);

	/**
	*   Returns type of activity
	*/
	ActivityWidget::ActivityType getActivityType() {
		return m_ActivityType;
	}

	/**
	*     Sets type of activity.
	*/
	void setActivityType( ActivityWidget::ActivityType activityType ) {
		m_ActivityType = activityType;
	}

	/**
	*   Returns the name of the activity.
	*/
	QString getName() {
		return m_Name;
	}

	/**
	*   Sets the name of the activity.
	*/
	void setName( QString name ) {
		m_Name = name;
	}

	/**
	*   Returns the documentation of the activity.
	*/
	QString getDoc() {
		return m_Doc;
	}

	/**
	*   Sets the documenation of the activity.
	*/
	void setDoc( QString doc ) {
		m_Doc = doc;
	}


	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	bool loadFromXMI( QDomElement & qElement );
private:
	/**
	*   Type of activity.
	*/
	ActivityWidget::ActivityType m_ActivityType;

	/**
	*   Name of the activity.
	*/
	QString m_Name;

	/**
	*   Documentation for the activity;
	*/
	QString m_Doc;

};

#endif
