/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef STATEWIDGETDATA_H
#define STATEWIDGETDATA_H
//qt includes
#include <qstringlist.h>

//app includes
#include "umlwidgetdata.h"
#include "statewidget.h"
#include "dialogs/settingsdlg.h"

/**
 *  This class holds all the StateWidget's Information
 *   All this information goes to a file or clipboard
 *   when an StateWidget object is saved
 *   With this class we are trying to achieve isolation
 *   between data and display layers.
 *   @author Paul Hensgen
 */
class StateWidgetData : public UMLWidgetData {
public:

	/**
	 *   Default constructor
	 */
	StateWidgetData(SettingsDlg::OptionState optionState);

	/**
	 *   Copy constructor
	 */
	StateWidgetData(StateWidgetData & Other);

	/**
	 *   Deconstructor
	 */
	virtual ~StateWidgetData();

	/**
	 *   Overrides '=' operator
	 */
	virtual StateWidgetData & operator=(StateWidgetData & Other);

	/**
	 *   Overrides '==' operator
	 */
	virtual bool operator==(StateWidgetData & Other);

	/**
	 *   Returns type of state
	 */
	StateWidget::StateType getStateType() {
		return m_StateType;
	}

	/**
	 *     Sets type of state.
	 */
	void setStateType( StateWidget::StateType stateType ) {
		m_StateType = stateType;
	}

	/**
	 *   Returns the name of the state.
	 */
	QString getName() {
		return m_Name;
	}

	/**
	 *   Sets the name of the state.
	 */
	void setName( QString name ) {
		m_Name = name;
	}

	/**
	 *   Returns the documentation of the state.
	 */
	QString getDoc() {
		return m_Doc;
	}

	/**
	 *   Sets the documenation of the state.
	 */
	void setDoc( QString doc ) {
		m_Doc = doc;
	}

	/**
	 *		Adds the given activity to the state.
	 */
	bool addActivity( QString activity );

	/**
	 *		Removes the given activity from the state.
	 */
	bool removeActivity( QString activity );

	/**
	 *		Returns the amount of activities in state.
	 */
	int getActivityCount() {
		return m_Activities.count();
	}

	/**
	 *		Returns the list of activities.
	 */
	QStringList & getActivityList() {
		return m_Activities;
	}

	/**
	 *		Sets the states activities to the ones given.
	 */
	void setActivities( QStringList & list );

	/**
	 *		Renames the given activity.
	 */
	bool renameActivity( QString activity, QString newName );

	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	bool loadFromXMI( QDomElement & qElement );
private:
	/**
	 *   Type of state.
	 */
	StateWidget::StateType m_StateType;

	/**
	 *   Name of the state.
	 */
	QString m_Name;

	/**
	 *   Documentation for the state;
	 */
	QString m_Doc;

	/**
	 *	List of activities for the state.
	 */
	QStringList m_Activities;
};

#endif
