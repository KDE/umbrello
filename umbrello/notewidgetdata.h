/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef NOTEWIDGETDATA_H
#define NOTEWIDGETDATA_H

#include <qstring.h>
#include "umlwidgetdata.h"
#include "dialogs/settingsdlg.h"

class QDataStream;

/**	This class holds all the NoteWidget's Information
 *	All this information goes to a file or clipboard
 *	when an NoteWidget object is saved
 *	With this class we are trying to achieve isolation
 *	between data and display layers.
 *	@author Gustavo Madrigal
 */

class NoteWidgetData : public UMLWidgetData {
public:
	/**
	 *  Standard constructor sets the base type
	 */
	NoteWidgetData(SettingsDlg::OptionState optionState);

	/**
	 * Copy constructor
	 */
	NoteWidgetData(NoteWidgetData & Other);

	/**
	 *  Empty destructor
	 */
	~NoteWidgetData();

	/**
	*
	*/
	virtual NoteWidgetData & operator=(NoteWidgetData & Other);

	/**
	*
	*/
	virtual bool operator==(NoteWidgetData & Other);

	/**
	*	Write property of QString m_Text.
	*/
	virtual void setText( QString Text);

	/**
	*	Read property of QString m_Text.
	*/
	virtual QString getText();

	/**
	*	Write property of bool m_bLinkDocumentation.
	*/
	virtual void setLinkDocumentation( bool LinkDocumentation);

	/**
	*	Read property of bool m_bLinkDocumentation.
	*/
	virtual bool getLinkDocumentation();

	/**
	 *	Saves the box to XMI using <UML:NoteWidget>
	 */
	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 *	Loads the <UML:NoteWidget> element
	 */
	bool loadFromXMI( QDomElement & qElement );
protected:
	/**
	*
	*/
	QString m_Text;

	/**
	*
	*/
	bool m_bLinkDocumentation;
};

#endif
